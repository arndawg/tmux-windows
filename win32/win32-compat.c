/*
 * POSIX compatibility shims for Windows.
 * Implements functions declared in win32-platform.h.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include "win32-platform.h"

/*
 * MSVC CRT provides _environ; POSIX code expects environ.
 * We provide the symbol and an init function to wire it up.
 */
char **environ = NULL;

void
win32_init_environ(void)
{
	environ = _environ;
}

/* flock() using LockFileEx/UnlockFileEx. */
int
win32_flock(int fd, int operation)
{
	HANDLE h;
	OVERLAPPED ov;
	DWORD flags = 0;

	h = (HANDLE)_get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE) {
		errno = EBADF;
		return (-1);
	}

	memset(&ov, 0, sizeof ov);

	if (operation & LOCK_UN)
		return UnlockFileEx(h, 0, MAXDWORD, MAXDWORD, &ov) ? 0 : -1;

	if (operation & LOCK_EX)
		flags |= LOCKFILE_EXCLUSIVE_LOCK;
	if (operation & LOCK_NB)
		flags |= LOCKFILE_FAIL_IMMEDIATELY;

	if (!LockFileEx(h, flags, 0, MAXDWORD, MAXDWORD, &ov)) {
		if (GetLastError() == ERROR_LOCK_VIOLATION)
			errno = EAGAIN;
		else
			errno = EIO;
		return (-1);
	}
	return (0);
}

/* closefrom: close all fds >= lowfd. */
void
win32_closefrom(int lowfd)
{
	int maxfd, fd;

	maxfd = _getmaxstdio();
	if (maxfd < 256)
		maxfd = 256;
	for (fd = lowfd; fd < maxfd; fd++)
		_close(fd); /* ignore errors */
}

/* clock_gettime using QueryPerformanceCounter. */
int
win32_clock_gettime(int clk_id, struct timespec *tp)
{
	static LARGE_INTEGER freq;
	static int freq_init = 0;
	LARGE_INTEGER counter;
	FILETIME ft;
	ULARGE_INTEGER uli;

	if (clk_id == CLOCK_MONOTONIC) {
		if (!freq_init) {
			QueryPerformanceFrequency(&freq);
			freq_init = 1;
		}
		QueryPerformanceCounter(&counter);
		tp->tv_sec = (time_t)(counter.QuadPart / freq.QuadPart);
		tp->tv_nsec = (long)(((counter.QuadPart % freq.QuadPart) *
		    1000000000LL) / freq.QuadPart);
		return (0);
	}

	/* CLOCK_REALTIME */
	GetSystemTimeAsFileTime(&ft);
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	/* Windows FILETIME epoch is 1601-01-01, Unix is 1970-01-01. */
	uli.QuadPart -= 116444736000000000ULL;
	tp->tv_sec = (time_t)(uli.QuadPart / 10000000ULL);
	tp->tv_nsec = (long)((uli.QuadPart % 10000000ULL) * 100);
	return (0);
}

/* gettimeofday. */
int
win32_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	ULARGE_INTEGER uli;

	(void)tz;
	GetSystemTimeAsFileTime(&ft);
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	uli.QuadPart -= 116444736000000000ULL;
	tv->tv_sec = (long)(uli.QuadPart / 10000000ULL);
	tv->tv_usec = (long)((uli.QuadPart % 10000000ULL) / 10);
	return (0);
}

/* getpwuid. */
static struct passwd pw_entry;
static char pw_name_buf[256];
static char pw_dir_buf[MAX_PATH];

struct passwd *
win32_getpwuid(uid_t uid)
{
	DWORD size;

	(void)uid;

	size = sizeof pw_name_buf;
	if (!GetUserNameA(pw_name_buf, &size))
		strncpy(pw_name_buf, "user", sizeof pw_name_buf);

	if (GetEnvironmentVariableA("USERPROFILE", pw_dir_buf,
	    sizeof pw_dir_buf) == 0)
		strncpy(pw_dir_buf, "C:\\", sizeof pw_dir_buf);

	pw_entry.pw_name = pw_name_buf;
	pw_entry.pw_dir = pw_dir_buf;
	pw_entry.pw_shell = _PATH_BSHELL;
	pw_entry.pw_uid = 0;
	pw_entry.pw_gid = 0;

	return (&pw_entry);
}

/* getprogname. */
static char progname_buf[MAX_PATH];
static int progname_init = 0;

const char *
win32_getprogname(void)
{
	char *p;

	if (!progname_init) {
		GetModuleFileNameA(NULL, progname_buf, sizeof progname_buf);
		/* Extract basename. */
		p = strrchr(progname_buf, '\\');
		if (p != NULL) {
			memmove(progname_buf, p + 1,
			    strlen(p + 1) + 1);
		}
		/* Remove .exe extension. */
		p = strrchr(progname_buf, '.');
		if (p != NULL && _stricmp(p, ".exe") == 0)
			*p = '\0';
		progname_init = 1;
	}
	return (progname_buf);
}

/* waitpid: dequeue from our exit tracking. */
struct win32_child_exit {
	pid_t pid;
	int   status;
	struct win32_child_exit *next;
};
static struct win32_child_exit *child_exit_list = NULL;
static CRITICAL_SECTION child_exit_lock;
static int child_exit_init_done = 0;

static void
child_exit_ensure_init(void)
{
	if (!child_exit_init_done) {
		InitializeCriticalSection(&child_exit_lock);
		child_exit_init_done = 1;
	}
}

void
win32_child_exited(pid_t pid, int status)
{
	struct win32_child_exit *e;

	child_exit_ensure_init();
	e = calloc(1, sizeof *e);
	if (e == NULL)
		return;
	e->pid = pid;
	e->status = status;

	EnterCriticalSection(&child_exit_lock);
	e->next = child_exit_list;
	child_exit_list = e;
	LeaveCriticalSection(&child_exit_lock);
}

pid_t
win32_waitpid(pid_t pid, int *status, int options)
{
	struct win32_child_exit **pp, *e;

	(void)options;
	child_exit_ensure_init();

	EnterCriticalSection(&child_exit_lock);
	for (pp = &child_exit_list; *pp != NULL; pp = &(*pp)->next) {
		e = *pp;
		if (pid == WAIT_ANY || e->pid == pid) {
			*pp = e->next;
			LeaveCriticalSection(&child_exit_lock);
			if (status != NULL)
				*status = e->status;
			pid = e->pid;
			free(e);
			return (pid);
		}
	}
	LeaveCriticalSection(&child_exit_lock);

	errno = ECHILD;
	return ((pid_t)-1);
}

/* kill: send TerminateProcess. */
int
win32_kill(pid_t pid, int sig)
{
	HANDLE h;

	if (sig == 0) {
		/* Check if process exists. */
		h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (h == NULL) {
			errno = ESRCH;
			return (-1);
		}
		CloseHandle(h);
		return (0);
	}

	if (sig == SIGTERM || sig == SIGKILL || sig == SIGHUP) {
		h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		if (h == NULL) {
			errno = EPERM;
			return (-1);
		}
		TerminateProcess(h, 1);
		CloseHandle(h);
		return (0);
	}

	/* Other signals: ignore. */
	return (0);
}

/* socketpair: create connected TCP pair via localhost. */
int
win32_socketpair(int domain, int type, int protocol, int sv[2])
{
	SOCKET listener, s1, s2;
	struct sockaddr_in addr;
	int addrlen = sizeof addr;

	(void)domain;
	(void)type;
	(void)protocol;

	win32_wsa_init();

	listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET)
		return (-1);

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0; /* ephemeral */

	if (bind(listener, (struct sockaddr *)&addr, sizeof addr) != 0)
		goto fail;
	if (getsockname(listener, (struct sockaddr *)&addr, &addrlen) != 0)
		goto fail;
	if (listen(listener, 1) != 0)
		goto fail;

	s1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s1 == INVALID_SOCKET)
		goto fail;
	if (connect(s1, (struct sockaddr *)&addr, sizeof addr) != 0) {
		closesocket(s1);
		goto fail;
	}

	s2 = accept(listener, NULL, NULL);
	if (s2 == INVALID_SOCKET) {
		closesocket(s1);
		goto fail;
	}

	closesocket(listener);

	sv[0] = (int)s1;
	sv[1] = (int)s2;
	return (0);

fail:
	closesocket(listener);
	return (-1);
}

/* getpeereid: always return uid/gid 0 on Windows. */
int
win32_getpeereid(int fd, uid_t *uid, gid_t *gid)
{
	(void)fd;
	*uid = 0;
	*gid = 0;
	return (0);
}

/* sendmsg: flatten iovecs and send. FD passing is a no-op. */
ssize_t
win32_sendmsg(int fd, const struct msghdr *msg, int flags)
{
	ssize_t total = 0;
	int i;

	for (i = 0; i < msg->msg_iovlen; i++) {
		ssize_t n = send((SOCKET)fd, (const char *)msg->msg_iov[i].iov_base,
		    (int)msg->msg_iov[i].iov_len, flags);
		if (n < 0) {
			if (total > 0)
				return (total);
			errno = WSAGetLastError();
			return (-1);
		}
		total += n;
		if ((size_t)n < msg->msg_iov[i].iov_len)
			break;
	}
	return (total);
}

/* recvmsg: receive into first iovec. No FD passing. */
ssize_t
win32_recvmsg(int fd, struct msghdr *msg, int flags)
{
	ssize_t n;

	if (msg->msg_iovlen < 1)
		return (0);

	n = recv((SOCKET)fd, (char *)msg->msg_iov[0].iov_base,
	    (int)msg->msg_iov[0].iov_len, flags);
	if (n < 0)
		errno = WSAGetLastError();

	/* No control message data on Windows. */
	msg->msg_controllen = 0;
	return (n);
}

#ifndef HAVE_STRSEP
char *
strsep(char **stringp, const char *delim)
{
	char *s, *tok;
	const char *spanp;
	int c, sc;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
}
#endif

/* getpwnam: look up user by name; just return current user info. */
struct passwd *
win32_getpwnam(const char *name)
{
	(void)name;
	return win32_getpwuid(0);
}

/* mkstemp: create and open a unique temporary file. */
int
win32_mkstemp(char *tmpl)
{
	if (_mktemp_s(tmpl, strlen(tmpl) + 1) != 0)
		return (-1);
	return _open(tmpl, _O_CREAT | _O_EXCL | _O_RDWR,
	    _S_IREAD | _S_IWRITE);
}

/* wcwidth: return display width of a wide character. */
int
win32_wcwidth(wchar_t wc)
{
	if (wc == 0)
		return (0);
	if (wc < 32 || (wc >= 0x7f && wc < 0xa0))
		return (-1);
	/* CJK and fullwidth: rough check. */
	if ((wc >= 0x1100 && wc <= 0x115f) ||
	    wc == 0x2329 || wc == 0x232a ||
	    (wc >= 0x2e80 && wc <= 0xa4cf && wc != 0x303f) ||
	    (wc >= 0xac00 && wc <= 0xd7a3) ||
	    (wc >= 0xf900 && wc <= 0xfaff) ||
	    (wc >= 0xfe10 && wc <= 0xfe19) ||
	    (wc >= 0xfe30 && wc <= 0xfe6f) ||
	    (wc >= 0xff01 && wc <= 0xff60) ||
	    (wc >= 0xffe0 && wc <= 0xffe6))
		return (2);
	return (1);
}

/*
 * fnmatch() implementation for Windows.
 * Supports *, ?, [...], FNM_CASEFOLD.
 */
static int
fnmatch_internal(const char *pat, const char *str, int flags)
{
	int pc, sc;

	for (;;) {
		pc = (unsigned char)*pat++;
		sc = (unsigned char)*str;

		switch (pc) {
		case '\0':
			return (sc == '\0') ? 0 : FNM_NOMATCH;

		case '?':
			if (sc == '\0')
				return FNM_NOMATCH;
			if ((flags & FNM_PATHNAME) && sc == '/')
				return FNM_NOMATCH;
			str++;
			break;

		case '*':
			/* Collapse multiple stars. */
			while (*pat == '*')
				pat++;
			if (*pat == '\0') {
				if (flags & FNM_PATHNAME) {
					return (strchr(str, '/') == NULL) ?
					    0 : FNM_NOMATCH;
				}
				return 0;
			}
			while (*str != '\0') {
				if (fnmatch_internal(pat, str, flags) == 0)
					return 0;
				if ((flags & FNM_PATHNAME) && *str == '/')
					break;
				str++;
			}
			return FNM_NOMATCH;

		case '[': {
			int negate = 0, match = 0;

			if (sc == '\0')
				return FNM_NOMATCH;

			if (*pat == '!' || *pat == '^') {
				negate = 1;
				pat++;
			}
			while (*pat != '\0') {
				if (*pat == ']') {
					pat++;
					break;
				}
				int lo = (unsigned char)*pat++;
				int hi = lo;
				if (*pat == '-' && pat[1] != ']' &&
				    pat[1] != '\0') {
					pat++; /* skip '-' */
					hi = (unsigned char)*pat++;
				}
				if (flags & FNM_CASEFOLD) {
					if (tolower(sc) >= tolower(lo) &&
					    tolower(sc) <= tolower(hi))
						match = 1;
				} else {
					if (sc >= lo && sc <= hi)
						match = 1;
				}
			}
			if (negate)
				match = !match;
			if (!match)
				return FNM_NOMATCH;
			str++;
			break;
		}

		default:
			if (flags & FNM_CASEFOLD) {
				if (tolower(pc) != tolower(sc))
					return FNM_NOMATCH;
			} else {
				if (pc != sc)
					return FNM_NOMATCH;
			}
			str++;
			break;
		}
	}
}

int
win32_fnmatch(const char *pattern, const char *string, int flags)
{
	return fnmatch_internal(pattern, string, flags);
}

/*
 * tparm: expand a terminfo parameterized string.
 *
 * Handles the common terminfo parameter operations:
 *   %p1 - %p9   push parameter N onto stack
 *   %d          pop and output as decimal
 *   %c          pop and output as character
 *   %s          pop and output as string
 *   %{nn}       push integer constant
 *   %i          increment p1 and p2 (for 1-based cursor addressing)
 *   %%          literal %
 *   Other characters are passed through literally.
 */
#define TPARM_STACK_SIZE 32
#define TPARM_BUF_SIZE   4096

char *
tparm(const char *str, ...)
{
	static char	 buf[TPARM_BUF_SIZE];
	intptr_t	 params[9];
	intptr_t	 stack[TPARM_STACK_SIZE];
	int		 sp = 0; /* stack pointer */
	size_t		 pos = 0;
	const char	*p;
	va_list		 ap;
	int		 i, n;

	if (str == NULL)
		return (NULL);

	va_start(ap, str);
	for (i = 0; i < 9; i++)
		params[i] = va_arg(ap, intptr_t);
	va_end(ap);

	memset(stack, 0, sizeof stack);

	for (p = str; *p != '\0' && pos < sizeof buf - 16; p++) {
		if (*p != '%') {
			buf[pos++] = *p;
			continue;
		}
		p++;
		if (*p == '\0')
			break;

		switch (*p) {
		case '%':
			buf[pos++] = '%';
			break;

		case 'p':
			/* Push parameter: %p1 through %p9. */
			p++;
			if (*p >= '1' && *p <= '9') {
				if (sp < TPARM_STACK_SIZE)
					stack[sp++] = params[*p - '1'];
			}
			break;

		case 'd':
			/* Pop and output as decimal. */
			if (sp > 0)
				n = (int)stack[--sp];
			else
				n = 0;
			pos += snprintf(buf + pos, sizeof buf - pos,
			    "%d", n);
			break;

		case 'c':
			/* Pop and output as character. */
			if (sp > 0)
				n = (int)stack[--sp];
			else
				n = 0;
			buf[pos++] = (char)n;
			break;

		case 's':
			/* Pop and output as string. */
			if (sp > 0) {
				const char *sv =
				    (const char *)(uintptr_t)stack[--sp];
				if (sv != NULL) {
					while (*sv != '\0' &&
					    pos < sizeof buf - 1)
						buf[pos++] = *sv++;
				}
			}
			break;

		case '{':
			/* Push integer constant: %{nn}. */
			p++;
			n = 0;
			while (*p >= '0' && *p <= '9') {
				n = n * 10 + (*p - '0');
				p++;
			}
			if (*p == '}') {
				if (sp < TPARM_STACK_SIZE)
					stack[sp++] = n;
			} else {
				/* Malformed, back up. */
				p--;
			}
			break;

		case 'i':
			/* Increment p1 and p2. */
			params[0]++;
			params[1]++;
			break;

		case '+':
			/* Pop two, push sum. */
			if (sp >= 2) {
				sp--;
				stack[sp - 1] += stack[sp];
			}
			break;

		case '-':
			/* Pop two, push difference. */
			if (sp >= 2) {
				sp--;
				stack[sp - 1] -= stack[sp];
			}
			break;

		case '*':
			/* Pop two, push product. */
			if (sp >= 2) {
				sp--;
				stack[sp - 1] *= stack[sp];
			}
			break;

		case '/':
			/* Pop two, push quotient. */
			if (sp >= 2) {
				sp--;
				if (stack[sp] != 0)
					stack[sp - 1] /= stack[sp];
			}
			break;

		case '?':
		case 't':
		case 'e':
		case ';':
			/*
			 * Conditional operations (%? %t %e %;).
			 * For simplicity, skip these -- the most common
			 * terminfo strings used by tmux don't rely on
			 * conditionals.
			 */
			break;

		default:
			/* Unknown sequence, output literally. */
			buf[pos++] = '%';
			if (pos < sizeof buf - 1)
				buf[pos++] = *p;
			break;
		}
	}

	buf[pos] = '\0';
	return (buf);
}
