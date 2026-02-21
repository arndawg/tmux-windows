/*
 * Signal emulation for Windows.
 * Uses SetConsoleCtrlHandler for SIGINT/SIGTERM,
 * a polling thread for SIGWINCH (console resize detection),
 * and a self-pipe for delivering signals into the libevent loop.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32-platform.h"

static SOCKET signal_pipe[2] = { INVALID_SOCKET, INVALID_SOCKET };
static HANDLE resize_thread = NULL;
static volatile int signal_shutdown = 0;
static SHORT last_cols = 0, last_rows = 0;

/* The signal callback registered with tmux's proc layer. */
static void (*signal_callback)(int) = NULL;

static BOOL WINAPI
ctrl_handler(DWORD type)
{
	switch (type) {
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
		win32_signal_notify(SIGINT);
		return TRUE;
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		win32_signal_notify(SIGTERM);
		return TRUE;
	}
	return FALSE;
}

static DWORD WINAPI
resize_poll_thread(LPVOID arg)
{
	HANDLE hConsole;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	(void)arg;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (!signal_shutdown) {
		if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
			SHORT cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			SHORT rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			if ((cols != last_cols || rows != last_rows) &&
			    last_cols != 0) {
				last_cols = cols;
				last_rows = rows;
				win32_signal_notify(SIGWINCH);
			} else {
				last_cols = cols;
				last_rows = rows;
			}
		}
		Sleep(100);
	}
	return 0;
}

void
win32_signal_init(void)
{
	int sv[2];
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hConsole;

	win32_wsa_init();

	if (win32_socketpair(AF_INET, SOCK_STREAM, 0, sv) == 0) {
		signal_pipe[0] = (SOCKET)sv[0];
		signal_pipe[1] = (SOCKET)sv[1];
	}

	SetConsoleCtrlHandler(ctrl_handler, TRUE);

	/* Initialize last known size. */
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
		last_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		last_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}

	signal_shutdown = 0;
	resize_thread = CreateThread(NULL, 0, resize_poll_thread, NULL, 0, NULL);
}

void
win32_signal_cleanup(void)
{
	signal_shutdown = 1;

	if (resize_thread != NULL) {
		WaitForSingleObject(resize_thread, 2000);
		CloseHandle(resize_thread);
		resize_thread = NULL;
	}

	SetConsoleCtrlHandler(ctrl_handler, FALSE);

	if (signal_pipe[0] != INVALID_SOCKET) {
		closesocket(signal_pipe[0]);
		closesocket(signal_pipe[1]);
		signal_pipe[0] = signal_pipe[1] = INVALID_SOCKET;
	}
}

int
win32_signal_get_fd(void)
{
	return (int)signal_pipe[0];
}

void
win32_signal_notify(int signo)
{
	char c = (char)signo;

	if (signal_pipe[1] != INVALID_SOCKET)
		send(signal_pipe[1], &c, 1, 0);
}

void
win32_signal_set_callback(void (*cb)(int))
{
	signal_callback = cb;
}

/*
 * Called from the event loop when the signal pipe is readable.
 * Reads signal numbers and dispatches them.
 */
void
win32_signal_dispatch(void)
{
	char buf[32];
	int n, i;

	if (signal_pipe[0] == INVALID_SOCKET)
		return;

	n = recv(signal_pipe[0], buf, sizeof buf, 0);
	if (n <= 0)
		return;

	for (i = 0; i < n; i++) {
		if (signal_callback != NULL)
			signal_callback((int)buf[i]);
	}
}
