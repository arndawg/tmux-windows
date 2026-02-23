/*
 * Windows platform compatibility header for tmux.
 * Provides POSIX type mappings, constants, signal stubs, and
 * struct definitions needed to compile tmux on Windows.
 */

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

/* Must be defined before including windows.h for newer APIs. */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 /* Windows 10 */
#endif
#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000007 /* Windows 10 1809+ for ConPTY */
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef SIZE
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/*
 * Undefine Windows macros that conflict with tmux identifiers.
 * - 'environ': MSVC expands to (*__p__environ()), tmux uses as struct member
 * - 'ERROR': wingdi.h defines as 0, tmux parser uses as token enum value
 */
#ifdef environ
#undef environ
#endif
#ifdef ERROR
#undef ERROR
#endif

/* Prevent redefinition conflicts. */
#ifdef _MSC_VER
#pragma warning(disable: 4005) /* macro redefinition */
#pragma warning(disable: 4018) /* signed/unsigned mismatch */
#pragma warning(disable: 4244) /* conversion, possible loss */
#pragma warning(disable: 4267) /* size_t to int */
#pragma warning(disable: 4996) /* deprecated POSIX names */
#endif

/*
 * POSIX type mappings.
 */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#ifdef _WIN64
typedef long long ssize_t;
#else
typedef long ssize_t;
#endif
#endif

#ifndef _PID_T_DEFINED
#define _PID_T_DEFINED
typedef int pid_t;
#endif

#ifndef _UID_T_DEFINED
#define _UID_T_DEFINED
typedef unsigned int uid_t;
#endif

#ifndef _GID_T_DEFINED
#define _GID_T_DEFINED
typedef unsigned int gid_t;
#endif

#ifndef _MODE_T_DEFINED
#define _MODE_T_DEFINED
typedef unsigned short mode_t;
#endif

typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;
typedef unsigned long u_long;

/*
 * POSIX constants.
 */
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

/* File mode bits (Windows doesn't have Unix permission model). */
#ifndef S_IRWXU
#define S_IRWXU 0700
#endif
#ifndef S_IRUSR
#define S_IRUSR 0400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0200
#endif
#ifndef S_IXUSR
#define S_IXUSR 0100
#endif
#ifndef S_IRWXG
#define S_IRWXG 0070
#endif
#ifndef S_IRGRP
#define S_IRGRP 0040
#endif
#ifndef S_IWGRP
#define S_IWGRP 0020
#endif
#ifndef S_IXGRP
#define S_IXGRP 0010
#endif
#ifndef S_IRWXO
#define S_IRWXO 0007
#endif
#ifndef S_IROTH
#define S_IROTH 0004
#endif
#ifndef S_IWOTH
#define S_IWOTH 0002
#endif
#ifndef S_IXOTH
#define S_IXOTH 0001
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif

/* Open flags. */
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif
#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

/* Access check. */
#ifndef X_OK
#define X_OK 0 /* Windows doesn't have execute permission bit */
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef F_OK
#define F_OK 0
#endif

/* fcntl constants (not used on Windows, but needed to compile). */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif
#ifndef F_SETFD
#define F_SETFD 2
#endif
#ifndef F_GETFD
#define F_GETFD 1
#endif
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

/* Wait macros. */
#ifndef WIFEXITED
#define WIFEXITED(status) (((status) & 0x7f) == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(status) (((status) >> 8) & 0xff)
#endif
#ifndef WIFSIGNALED
#define WIFSIGNALED(status) (((status) & 0x7f) != 0 && ((status) & 0x7f) != 0x7f)
#endif
#ifndef WTERMSIG
#define WTERMSIG(status) ((status) & 0x7f)
#endif
#ifndef WIFSTOPPED
#define WIFSTOPPED(status) (0)
#endif
#ifndef WSTOPSIG
#define WSTOPSIG(status) (0)
#endif
#ifndef WNOHANG
#define WNOHANG 1
#endif
#ifndef WUNTRACED
#define WUNTRACED 2
#endif
#ifndef WAIT_ANY
#define WAIT_ANY -1
#endif

/*
 * Signal number stubs.
 * Windows only has SIGINT, SIGTERM, SIGABRT, SIGSEGV, SIGFPE, SIGILL.
 * We define the rest as unique numbers for our emulation layer.
 */
#ifndef SIGHUP
#define SIGHUP    1
#endif
#ifndef SIGQUIT
#define SIGQUIT   3
#endif
#ifndef SIGTRAP
#define SIGTRAP   5
#endif
#ifndef SIGPIPE
#define SIGPIPE   13
#endif
#ifndef SIGALRM
#define SIGALRM   14
#endif
#ifndef SIGCHLD
#define SIGCHLD   17
#endif
#ifndef SIGCONT
#define SIGCONT   18
#endif
#ifndef SIGTSTP
#define SIGTSTP   20
#endif
#ifndef SIGTTIN
#define SIGTTIN   21
#endif
#ifndef SIGTTOU
#define SIGTTOU   22
#endif
#ifndef SIGUSR1
#define SIGUSR1   10
#endif
#ifndef SIGUSR2
#define SIGUSR2   12
#endif
#ifndef SIGKILL
#define SIGKILL   9
#endif
#ifndef SIGWINCH
#define SIGWINCH  28
#endif
#ifndef NSIG
#define NSIG      32
#endif

/* sigaction stub. */
struct sigaction {
	void	(*sa_handler)(int);
	int	  sa_flags;
	int	  sa_mask;
};
#define SA_RESTART 0
#define SIG_DFL ((void (*)(int))0)
#define SIG_IGN ((void (*)(int))1)

typedef int sigset_t;
static inline int sigemptyset(sigset_t *set) { *set = 0; return 0; }
static inline int sigfillset(sigset_t *set) { *set = ~0; return 0; }
static inline int sigaddset(sigset_t *set, int signo) { (void)set; (void)signo; return 0; }
static inline int sigdelset(sigset_t *set, int signo) { (void)set; (void)signo; return 0; }
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
static inline int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
	(void)how; (void)set;
	if (oldset) *oldset = 0;
	return 0;
}
static inline int sigaction(int signo, const struct sigaction *act,
    struct sigaction *oldact) {
	(void)signo; (void)act; (void)oldact;
	return 0;
}
static inline const char *strsignal(int signo) {
	switch (signo) {
	case SIGINT: return "Interrupt";
	case SIGTERM: return "Terminated";
	case SIGHUP: return "Hangup";
	case SIGCHLD: return "Child exited";
	case SIGWINCH: return "Window changed";
	case SIGCONT: return "Continued";
	case SIGUSR1: return "User signal 1";
	case SIGUSR2: return "User signal 2";
	default: return "Unknown signal";
	}
}

/*
 * struct termios stub.
 * On Windows, terminal control is done via Console API.
 */
#ifndef HAVE_TERMIOS
typedef unsigned char cc_t;
#define NCCS 20
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif
struct termios {
	unsigned int c_iflag;
	unsigned int c_oflag;
	unsigned int c_cflag;
	unsigned int c_lflag;
	unsigned char c_cc[NCCS];
	unsigned int c_ispeed;
	unsigned int c_ospeed;
};

/* termios c_iflag bits. */
#define IGNBRK  0000001
#define BRKINT  0000002
#define IGNPAR  0000004
#define PARMRK  0000010
#define INPCK   0000020
#define ISTRIP  0000040
#define INLCR   0000100
#define IGNCR   0000200
#define ICRNL   0000400
#define IXON    0002000
#define IXANY   0004000
#define IXOFF   0010000
#define IUTF8   0040000

/* termios c_oflag bits. */
#define OPOST   0000001
#define ONLCR   0000004

/* termios c_cflag bits. */
#define CREAD   0000200
#define CS8     0000060
#define HUPCL   0002000

/* termios c_lflag bits. */
#define ECHO    0000010
#define ECHOE   0000020
#define ECHOK   0000040
#define ECHONL  0000100
#define ISIG    0000001
#define ICANON  0000002
#define IEXTEN  0100000
#define NOFLSH  0000200

/* termios c_cc indexes. */
#define VEOF    4
#define VEOL    5
#define VERASE  2
#define VINTR   0
#define VKILL   3
#define VMIN    6
#define VQUIT   1
#define VTIME   7

/* termios actions. */
#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

/* Baud rates (stubs). */
#define B0      0
#define B9600   9600
#define B38400  38400

static inline int tcgetattr(int fd, struct termios *t) {
	(void)fd;
	memset(t, 0, sizeof *t);
	return 0;
}
static inline int tcsetattr(int fd, int act, const struct termios *t) {
	(void)fd; (void)act; (void)t;
	return 0;
}
static inline int tcflush(int fd, int queue) {
	(void)fd; (void)queue;
	return 0;
}
static inline void cfmakeraw(struct termios *t) {
	memset(t, 0, sizeof *t);
}
static inline int cfgetispeed(const struct termios *t) {
	return (int)t->c_ispeed;
}
static inline int cfgetospeed(const struct termios *t) {
	return (int)t->c_ospeed;
}
static inline int cfsetispeed(struct termios *t, int speed) {
	t->c_ispeed = speed; return 0;
}
static inline int cfsetospeed(struct termios *t, int speed) {
	t->c_ospeed = speed; return 0;
}
#define HAVE_CFMAKERAW 1
#endif /* HAVE_TERMIOS */

/*
 * struct winsize stub.
 */
struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

/* ioctl stub for TIOCSWINSZ/TIOCGWINSZ (handled by our ConPTY layer). */
#define TIOCSWINSZ 0x5414
#define TIOCGWINSZ 0x5413
#define FIONREAD   0x541B

/*
 * struct iovec for readv/writev emulation.
 */
#ifndef _SYS_UIO_H
struct iovec {
	void  *iov_base;
	size_t iov_len;
};

#ifndef IOV_MAX
#define IOV_MAX 16
#endif

static inline ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
	ssize_t total = 0;
	int i;
	for (i = 0; i < iovcnt; i++) {
		ssize_t n = _read(fd, iov[i].iov_base, (unsigned int)iov[i].iov_len);
		if (n < 0) return total > 0 ? total : -1;
		total += n;
		if ((size_t)n < iov[i].iov_len) break;
	}
	return total;
}

static inline ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
	ssize_t total = 0;
	int i;
	for (i = 0; i < iovcnt; i++) {
		ssize_t n = _write(fd, iov[i].iov_base, (unsigned int)iov[i].iov_len);
		if (n < 0) return total > 0 ? total : -1;
		total += n;
		if ((size_t)n < iov[i].iov_len) break;
	}
	return total;
}
#endif /* _SYS_UIO_H */

/*
 * Path constants.
 */
#undef _PATH_BSHELL
#define _PATH_BSHELL "cmd.exe"

#undef _PATH_TMP
#define _PATH_TMP "C:\\Windows\\Temp\\"

#undef _PATH_DEVNULL
#define _PATH_DEVNULL "NUL"

#undef _PATH_TTY
#define _PATH_TTY "CON"

#undef _PATH_DEV
#define _PATH_DEV ""

#undef _PATH_DEFPATH
#define _PATH_DEFPATH ""

#undef _PATH_VI
#define _PATH_VI "notepad.exe"

/*
 * Default tmux paths for Windows.
 */
#undef TMUX_CONF
#define TMUX_CONF "~/.tmux.conf"

#undef TMUX_SOCK
#define TMUX_SOCK "$LOCALAPPDATA/tmux"

/*
 * POSIX function mappings.
 */
#define getpid()       ((pid_t)GetCurrentProcessId())
#define getppid()      ((pid_t)0)
#define getuid()       ((uid_t)0)
#define getgid()       ((gid_t)0)
#define geteuid()      ((uid_t)0)
#define getegid()      ((gid_t)0)
#define setsid()       (0)
#define umask(m)       ((mode_t)0)
#define chmod(p, m)    (0)
#define chown(p, u, g) (0)
#define lstat(p, b)    stat(p, b)
#define link(o, n)     (0)
#define symlink(o, n)  (0)
#define readlink(p, b, s) (-1)
#define kill(pid, sig) win32_kill(pid, sig)
#define killpg(pid, sig) win32_kill(pid, sig)
#define chdir(p)       _chdir(p)
#define getcwd(b, s)   _getcwd(b, (int)(s))
#define mkdir(p, m)    _mkdir(p)
#define rmdir(p)       _rmdir(p)
#define unlink(p)      _unlink(p)
#define access(p, m)   _access(p, m)
#define open(p, ...)   _open(p, __VA_ARGS__)
#define close(fd)      _close(fd)
#define read(fd, b, n) _read(fd, b, (unsigned int)(n))
#define write(fd, b, n) _write(fd, b, (unsigned int)(n))
#define dup(fd)        _dup(fd)
#define dup2(o, n)     _dup2(o, n)
#define pipe(fds)      _pipe(fds, 4096, _O_BINARY)
#define fileno(f)      _fileno(f)
#define isatty(fd)     _isatty(fd)
#define lseek(fd, o, w) _lseek(fd, o, w)
#define ftruncate(fd, l) _chsize(fd, (long)(l))
#define realpath(p, r) _fullpath(r, p, PATH_MAX)
#define strcasecmp     _stricmp
#define strncasecmp    _strnicmp
#define strtok_r       strtok_s

/* POSIX-to-Windows process stubs (these are implemented in win32-process.c). */
int win32_kill(pid_t pid, int sig);

/*
 * setenv/unsetenv.
 */
#ifndef HAVE_SETENV
static inline int setenv(const char *name, const char *value, int overwrite) {
	if (!overwrite && getenv(name) != NULL) return 0;
	return _putenv_s(name, value);
}
static inline int unsetenv(const char *name) {
	return _putenv_s(name, "");
}
#define HAVE_SETENV 1
#endif

/*
 * fcntl stub (non-functional on Windows, but compiles).
 */
static inline int fcntl(int fd, int cmd, ...) {
	(void)fd; (void)cmd;
	return 0;
}

/* flock shims. */
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

int win32_flock(int fd, int operation);
#define flock(fd, op) win32_flock(fd, op)
#define HAVE_FLOCK 1

/*
 * Stubs for Unix functions that don't apply on Windows.
 */
#define pledge(s, p) (0)
#define setproctitle(...) do {} while (0)
#define HAVE_SETPROCTITLE 1

/* ttyname stub. */
static inline char *ttyname(int fd) {
	(void)fd;
	return "CON";
}

/* getpwuid stub. */
struct passwd {
	char *pw_name;
	char *pw_dir;
	char *pw_shell;
	uid_t pw_uid;
	gid_t pw_gid;
};
struct passwd *win32_getpwuid(uid_t uid);
#define getpwuid(uid) win32_getpwuid(uid)

/* gettimeofday. */
#ifndef HAVE_GETTIMEOFDAY
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
int win32_gettimeofday(struct timeval *tv, struct timezone *tz);
#define gettimeofday(tv, tz) win32_gettimeofday(tv, tz)
#define HAVE_GETTIMEOFDAY 1
#endif

/* clock_gettime. */
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME  0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
int win32_clock_gettime(int clk_id, struct timespec *tp);
#define clock_gettime(c, t) win32_clock_gettime(c, t)
#define HAVE_CLOCK_GETTIME 1

/* closefrom. */
void win32_closefrom(int lowfd);
#define closefrom(fd) win32_closefrom(fd)
#define HAVE_CLOSEFROM 1

/* getprogname. */
const char *win32_getprogname(void);
#define getprogname() win32_getprogname()
#define HAVE_GETPROGNAME 1

/* waitpid stub (handled by our process watcher). */
pid_t win32_waitpid(pid_t pid, int *status, int options);
#define waitpid(pid, s, o) win32_waitpid(pid, s, o)

/* daemon stub (handled by our process management). */
#define daemon(nochdir, noclose) (0)
#define HAVE_DAEMON 1

/* socketpair. */
int win32_socketpair(int domain, int type, int protocol, int sv[2]);
#define socketpair(d, t, p, sv) win32_socketpair(d, t, p, sv)

/* getpeereid. */
int win32_getpeereid(int fd, uid_t *uid, gid_t *gid);
#define getpeereid(fd, u, g) win32_getpeereid(fd, u, g)
#define HAVE_GETPEEREID 1

/*
 * Socket compat: make socket functions work with int fds.
 * On Windows, sockets are SOCKET (unsigned pointer), not int.
 * We use Winsock functions and treat the int as a SOCKET cast.
 */
/* Socket shutdown constants. */
#ifndef SHUT_RD
#define SHUT_RD   0
#endif
#ifndef SHUT_WR
#define SHUT_WR   1
#endif
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#define PF_UNSPEC 0
#ifndef AF_UNIX
#define AF_UNIX AF_INET /* redirect to TCP loopback */
#endif

/* strsep, strlcpy, strlcat may be missing. */
#ifndef HAVE_STRSEP
char *strsep(char **stringp, const char *delim);
#endif

/* uname stub. */
struct utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
};
static inline int uname(struct utsname *u) {
	memset(u, 0, sizeof *u);
	strncpy(u->sysname, "Windows", sizeof u->sysname - 1);
	strncpy(u->release, "10", sizeof u->release - 1);
	strncpy(u->version, "Native", sizeof u->version - 1);
#ifdef _M_X64
	strncpy(u->machine, "x86_64", sizeof u->machine - 1);
#elif defined(_M_ARM64)
	strncpy(u->machine, "aarch64", sizeof u->machine - 1);
#else
	strncpy(u->machine, "i686", sizeof u->machine - 1);
#endif
	{
		DWORD size = sizeof u->nodename;
		GetComputerNameA(u->nodename, &size);
	}
	return 0;
}

/* nl_langinfo / locale stubs. */
#define CODESET 0
static inline const char *nl_langinfo(int item) {
	(void)item;
	return "UTF-8";
}

/*
 * Errno values that Windows doesn't define but POSIX code uses.
 */
#ifndef ECHILD
#define ECHILD     10
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#endif
#ifndef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED
#endif
#ifndef ENOBUFS
#define ENOBUFS    WSAENOBUFS
#endif
#ifndef EMSGSIZE
#define EMSGSIZE   WSAEMSGSIZE
#endif
#ifndef ENOENT
/* Already defined in MSVC errno.h */
#endif

/* EBADMSG may be missing. */
#ifndef EBADMSG
#define EBADMSG 74
#endif

/* EOVERFLOW. */
#ifndef EOVERFLOW
#define EOVERFLOW 75
#endif

/*
 * Socket address: sockaddr_un stub to compile Unix socket code paths.
 * We never actually use AF_UNIX; we redirect to TCP. But some code
 * references struct sockaddr_un, so provide a minimal stub.
 */
#ifndef _SYS_UN_H
struct sockaddr_un {
	unsigned short sun_family;
	char sun_path[108];
};
#endif

/*
 * fnmatch replacement for Windows.
 */
#define FNM_NOMATCH    1
#define FNM_NOESCAPE   (1 << 1)
#define FNM_PATHNAME   (1 << 2)
#define FNM_PERIOD     (1 << 3)
#define FNM_CASEFOLD   (1 << 4)
int win32_fnmatch(const char *pattern, const char *string, int flags);
#define fnmatch(p, s, f) win32_fnmatch(p, s, f)

/*
 * regex.h replacement for Windows.
 */
#include "win32-regex.h"

/* NOKERNINFO may be referenced in client.c. */
#ifndef NOKERNINFO
#define NOKERNINFO 0
#endif

/* ECHOPRT for compat.h. */
#ifndef ECHOPRT
#define ECHOPRT 0
#endif

/* ECHOCTL for termios stubs. */
#ifndef ECHOCTL
#define ECHOCTL 0
#endif

/* ECHOKE for termios stubs. */
#ifndef ECHOKE
#define ECHOKE 0
#endif

/* ONLRET for termios stubs. */
#ifndef ONLRET
#define ONLRET 0
#endif

/* OCRNL for termios stubs. */
#ifndef OCRNL
#define OCRNL 0
#endif

/* TCOFLUSH for tcflush. */
#ifndef TCOFLUSH
#define TCOFLUSH 1
#endif

/* basename/dirname for Windows (libgen.h replacements). */
static inline char *win32_basename(char *path) {
	char *p;
	if (path == NULL || *path == '\0')
		return ".";
	p = path + strlen(path) - 1;
	while (p > path && (*p == '/' || *p == '\\'))
		*p-- = '\0';
	while (p > path && *p != '/' && *p != '\\')
		p--;
	if (*p == '/' || *p == '\\')
		p++;
	return p;
}
static inline char *win32_dirname(char *path) {
	static char dot[] = ".";
	char *p;
	if (path == NULL || *path == '\0')
		return dot;
	p = path + strlen(path) - 1;
	while (p > path && (*p == '/' || *p == '\\'))
		*p-- = '\0';
	while (p > path && *p != '/' && *p != '\\')
		p--;
	if (p == path) {
		if (*p == '/' || *p == '\\')
			path[1] = '\0';
		else
			return dot;
	} else {
		*p = '\0';
	}
	return path;
}
#define basename(p) win32_basename(p)
#define dirname(p) win32_dirname(p)

/* IMAXBEL for compat.h. */
#ifndef IMAXBEL
#define IMAXBEL 0
#endif

/*
 * Winsock initialization helper.
 */
static inline void win32_wsa_init(void) {
	static int initialized = 0;
	if (!initialized) {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
		initialized = 1;
	}
}

/*
 * Forward declarations for win32 module functions.
 */

/* win32-pty.c */
struct win32_pty;
struct win32_pty *win32_pty_spawn(const char *cmd, const char *cwd,
    char *env, int cols, int rows, pid_t *pid);
int  win32_pty_resize(struct win32_pty *pty, int cols, int rows);
void win32_pty_close(struct win32_pty *pty);
int  win32_pty_get_fd(struct win32_pty *pty);
HANDLE win32_pty_get_process(struct win32_pty *pty);

/* win32-process.c */
void  win32_process_watch(HANDLE hProcess, pid_t pid);
void  win32_process_unwatch(pid_t pid);
void  win32_process_init(void);
void  win32_process_cleanup(void);
pid_t win32_process_spawn(const char *cmd, const char *cwd, int outfd);
void  win32_launch_server(const char *label);

/* win32-ipc.c */
int  win32_ipc_create_server(const char *label, uint16_t *port);
int  win32_ipc_connect(const char *label);
int  win32_ipc_connect_tty(const char *label, const char *tty_token);
int  win32_ipc_verify_auth(int fd, const char *label, char *tty_token_out,
	 size_t tty_token_size);
void win32_ipc_cleanup(const char *label);
void win32_generate_tty_token(char *buf, size_t bufsize);

/* win32-signal.c */
void win32_signal_init(void);
void win32_signal_cleanup(void);
int  win32_signal_get_fd(void);
void win32_signal_notify(int signo);
void win32_signal_set_callback(void (*cb)(int));
void win32_signal_dispatch(void);

/* win32-tty.c */
int  win32_tty_raw_mode(void);
void win32_tty_restore(void);
int  win32_tty_get_size(int *cols, int *rows);
void win32_tty_init_utf8(void);

/* win32-terminfo.c */
struct win32_terminfo_entry {
	const char *name;
	const char *value;
	int         numeric;
	int         flag;
	int         type; /* 0=none, 1=string, 2=number, 3=flag */
};
const struct win32_terminfo_entry *win32_terminfo_table(u_int *count);
const struct win32_terminfo_entry *win32_terminfo_find(const char *name);
u_int win32_terminfo_count(void);

/* win32-compat.c */
/* (function declarations are above as #define targets) */
char *tparm(const char *str, ...);

/* execl / execvp stubs (not real exec; we use CreateProcess). */
#define execl(path, ...) (-1)
#define execvp(file, argv) (-1)
#define _exit(code) exit(code)

/* caddr_t. */
#ifndef caddr_t
typedef char *caddr_t;
#endif

/* sendmsg/recvmsg stubs for imsg-buffer.c compilation. */
struct msghdr {
	void         *msg_name;
	int           msg_namelen;
	struct iovec *msg_iov;
	int           msg_iovlen;
	void         *msg_control;
	int           msg_controllen;
	int           msg_flags;
};
/* struct cmsghdr is provided by ws2def.h (via winsock2.h) */
#ifndef SCM_RIGHTS
#define SCM_RIGHTS 1
#endif
#ifndef SOL_SOCKET
#define SOL_SOCKET 0xffff
#endif

#ifndef CMSG_FIRSTHDR
#define CMSG_FIRSTHDR(mhdr) \
	((mhdr)->msg_controllen >= (int)sizeof(struct cmsghdr) ? \
	    (struct cmsghdr *)(mhdr)->msg_control : (struct cmsghdr *)NULL)
#endif
#ifndef CMSG_NXTHDR
#define CMSG_NXTHDR(mhdr, cmsg) ((struct cmsghdr *)NULL)
#endif
#ifndef CMSG_DATA
#define CMSG_DATA(cmsg) ((unsigned char *)((cmsg) + 1))
#endif
#ifndef CMSG_ALIGN
#define CMSG_ALIGN(len) (((len) + sizeof(long) - 1) & ~(sizeof(long) - 1))
#endif
#ifndef CMSG_SPACE
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#endif
#ifndef CMSG_LEN
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))
#endif

/* sendmsg/recvmsg: on Windows we replace these with send/recv in imsg-buffer.c */
ssize_t win32_sendmsg(int fd, const struct msghdr *msg, int flags);
ssize_t win32_recvmsg(int fd, struct msghdr *msg, int flags);
#define sendmsg(fd, msg, flags) win32_sendmsg(fd, msg, flags)
#define recvmsg(fd, msg, flags) win32_recvmsg(fd, msg, flags)

/* System() is available on Windows. */

/* SUN_LEN stub. */
#ifndef SUN_LEN
#define SUN_LEN(sun) (sizeof(*(sun)))
#endif

/* ENAMETOOLONG. */
#ifndef ENAMETOOLONG
#define ENAMETOOLONG 36
#endif

/* INFTIM. */
#ifndef INFTIM
#define INFTIM -1
#endif

/* TTY_NAME_MAX. */
#ifndef TTY_NAME_MAX
#define TTY_NAME_MAX 32
#endif

/* HOST_NAME_MAX. */
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

/* ACCESSPERMS. */
#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#endif

/*
 * environ: POSIX code expects 'extern char **environ'.
 * We provide the symbol in win32-compat.c and initialize it
 * via win32_init_environ() which must be called early in main().
 */
extern char **environ;
void win32_init_environ(void);

/* getpwnam. */
struct passwd *win32_getpwnam(const char *name);
#define getpwnam(n) win32_getpwnam(n)

/*
 * POSIX thread-safe time functions.
 * MSVC provides _s variants with swapped argument order.
 */
static inline struct tm *localtime_r(const time_t *t, struct tm *result) {
	localtime_s(result, t);
	return result;
}
static inline struct tm *gmtime_r(const time_t *t, struct tm *result) {
	gmtime_s(result, t);
	return result;
}
static inline char *ctime_r(const time_t *t, char *buf) {
	ctime_s(buf, 26, t);
	return buf;
}

/* fseeko / ftello: 64-bit file offset support. */
#define fseeko(f, o, w) _fseeki64(f, o, w)
#define ftello(f) _ftelli64(f)

/* mkstemp. */
int win32_mkstemp(char *tmpl);
#define mkstemp(t) win32_mkstemp(t)

/* usleep: microsecond sleep (truncated to milliseconds on Windows). */
static inline int usleep(unsigned int usec) {
	Sleep(usec / 1000);
	return 0;
}

/* wcwidth. */
int win32_wcwidth(wchar_t wc);
#define wcwidth(wc) win32_wcwidth(wc)

/* getpagesize. */
static inline int getpagesize(void) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return (int)si.dwPageSize;
}

/* MSVC CRT provides strnlen; prevent compat.h from redeclaring it. */
#define HAVE_STRNLEN 1

/* Map TMUX_VERSION to PACKAGE_VERSION for tmux.c. */
#ifdef PACKAGE_VERSION
#ifndef TMUX_VERSION
#define TMUX_VERSION PACKAGE_VERSION
#endif
#endif

#endif /* WIN32_PLATFORM_H */
