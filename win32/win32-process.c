/*
 * Process management for Windows.
 * Tracks child process handles and notifies the event loop when they exit.
 * Replaces SIGCHLD / waitpid() functionality.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32-platform.h"

/* External: defined in win32-compat.c */
void win32_child_exited(pid_t pid, int status);

#define MAX_WATCHED 64

struct watched_process {
	HANDLE   hProcess;
	pid_t    pid;
	int      active;
};

static struct watched_process watched[MAX_WATCHED];
static int nwatched = 0;
static CRITICAL_SECTION watch_lock;
static HANDLE watcher_thread = NULL;
static HANDLE watcher_event = NULL; /* signaled when watch list changes */
static volatile int watcher_shutdown = 0;

static DWORD WINAPI
process_watcher_thread(LPVOID arg)
{
	HANDLE handles[MAX_WATCHED + 1];
	DWORD count, result;
	int i;

	(void)arg;

	for (;;) {
		if (watcher_shutdown)
			break;

		EnterCriticalSection(&watch_lock);
		count = 0;
		handles[count++] = watcher_event; /* always first */
		for (i = 0; i < nwatched; i++) {
			if (watched[i].active && count < MAX_WATCHED + 1)
				handles[count++] = watched[i].hProcess;
		}
		LeaveCriticalSection(&watch_lock);

		result = WaitForMultipleObjects(count, handles, FALSE, 1000);

		if (result == WAIT_OBJECT_0) {
			/* Watcher event signaled: list changed, re-loop. */
			ResetEvent(watcher_event);
			continue;
		}

		if (result == WAIT_TIMEOUT)
			continue;

		if (result >= WAIT_OBJECT_0 + 1 &&
		    result < WAIT_OBJECT_0 + count) {
			/* A child process exited. */
			DWORD idx = result - WAIT_OBJECT_0 - 1;
			DWORD exit_code = 0;
			pid_t pid;

			EnterCriticalSection(&watch_lock);
			/* Map back to watched array index. */
			DWORD wi = 0;
			for (i = 0; i < nwatched; i++) {
				if (watched[i].active) {
					if (wi == idx) {
						GetExitCodeProcess(watched[i].hProcess, &exit_code);
						pid = watched[i].pid;
						CloseHandle(watched[i].hProcess);
						watched[i].active = 0;

						/* Compact. */
						if (i == nwatched - 1)
							nwatched--;

						LeaveCriticalSection(&watch_lock);

						/* Encode exit code like Unix: (code << 8). */
						win32_child_exited(pid, (int)(exit_code << 8));

						/* Notify signal pipe (SIGCHLD). */
						win32_signal_notify(SIGCHLD);
						goto next;
					}
					wi++;
				}
			}
			LeaveCriticalSection(&watch_lock);
		}
next:;
	}
	return 0;
}

void
win32_process_init(void)
{
	InitializeCriticalSection(&watch_lock);
	watcher_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	watcher_shutdown = 0;
	nwatched = 0;
	watcher_thread = CreateThread(NULL, 0, process_watcher_thread, NULL, 0, NULL);
}

void
win32_process_cleanup(void)
{
	int i;

	watcher_shutdown = 1;
	if (watcher_event)
		SetEvent(watcher_event);
	if (watcher_thread) {
		WaitForSingleObject(watcher_thread, 5000);
		CloseHandle(watcher_thread);
		watcher_thread = NULL;
	}
	if (watcher_event) {
		CloseHandle(watcher_event);
		watcher_event = NULL;
	}

	EnterCriticalSection(&watch_lock);
	for (i = 0; i < nwatched; i++) {
		if (watched[i].active)
			CloseHandle(watched[i].hProcess);
	}
	nwatched = 0;
	LeaveCriticalSection(&watch_lock);
	DeleteCriticalSection(&watch_lock);
}

void
win32_process_watch(HANDLE hProcess, pid_t pid)
{
	HANDLE dup;

	/* Duplicate the handle so we own it. */
	if (!DuplicateHandle(GetCurrentProcess(), hProcess,
	    GetCurrentProcess(), &dup, 0, FALSE, DUPLICATE_SAME_ACCESS))
		return;

	EnterCriticalSection(&watch_lock);
	if (nwatched < MAX_WATCHED) {
		watched[nwatched].hProcess = dup;
		watched[nwatched].pid = pid;
		watched[nwatched].active = 1;
		nwatched++;
	} else {
		CloseHandle(dup);
	}
	LeaveCriticalSection(&watch_lock);

	/* Signal the watcher thread to re-read the list. */
	if (watcher_event)
		SetEvent(watcher_event);
}

/*
 * Spawn a non-PTY process with its stdout/stdin connected to the given socket.
 * Returns the child PID on success, -1 on failure.
 */
pid_t
win32_process_spawn(const char *cmd, const char *cwd, int outfd)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	HANDLE hOut;
	SECURITY_ATTRIBUTES sa;
	char *cmddup;

	memset(&sa, 0, sizeof sa);
	sa.nLength = sizeof sa;
	sa.bInheritHandle = TRUE;

	/* Convert the socket to a handle that can be inherited. */
	hOut = (HANDLE)_get_osfhandle(outfd);
	if (hOut == INVALID_HANDLE_VALUE)
		return (-1);

	memset(&si, 0, sizeof si);
	si.cb = sizeof si;
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hOut;
	si.hStdOutput = hOut;
	si.hStdError = hOut;

	memset(&pi, 0, sizeof pi);
	cmddup = _strdup(cmd);

	if (!CreateProcessA(NULL, cmddup, NULL, NULL, TRUE,
	    CREATE_NO_WINDOW, NULL,
	    (cwd != NULL && *cwd != '\0') ? cwd : NULL,
	    &si, &pi)) {
		free(cmddup);
		return (-1);
	}
	free(cmddup);

	CloseHandle(pi.hThread);

	/* Watch the process for exit. */
	win32_process_watch(pi.hProcess, (pid_t)pi.dwProcessId);
	CloseHandle(pi.hProcess);

	return ((pid_t)pi.dwProcessId);
}

/*
 * Launch a detached tmux server process.
 * Runs: tmux.exe -D -S "<label>"
 * The -D flag tells tmux to run as a foreground server (no fork).
 */
void
win32_launch_server(const char *label)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	char exe[MAX_PATH];
	char cmdline[MAX_PATH * 2];

	if (GetModuleFileNameA(NULL, exe, sizeof exe) == 0)
		return;

	snprintf(cmdline, sizeof cmdline,
	    "\"%s\" -D -S \"%s\"", exe, label);

	memset(&si, 0, sizeof si);
	si.cb = sizeof si;
	memset(&pi, 0, sizeof pi);

	/*
	 * CREATE_BREAKAWAY_FROM_JOB detaches the server from any job object
	 * the parent belongs to (e.g., sshd's session job). Without this,
	 * closing an SSH connection kills the tmux server along with all
	 * processes in the job. Fall back without the flag if the job
	 * doesn't allow breakaway (ERROR_ACCESS_DENIED).
	 */
	if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
	    DETACHED_PROCESS | CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB,
	    NULL, NULL, &si, &pi)) {
		if (GetLastError() != ERROR_ACCESS_DENIED)
			return;
		if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE,
		    DETACHED_PROCESS | CREATE_NO_WINDOW, NULL, NULL,
		    &si, &pi))
			return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

void
win32_process_unwatch(pid_t pid)
{
	int i;

	EnterCriticalSection(&watch_lock);
	for (i = 0; i < nwatched; i++) {
		if (watched[i].active && watched[i].pid == pid) {
			CloseHandle(watched[i].hProcess);
			watched[i].active = 0;
			if (i == nwatched - 1)
				nwatched--;
			break;
		}
	}
	LeaveCriticalSection(&watch_lock);

	if (watcher_event)
		SetEvent(watcher_event);
}
