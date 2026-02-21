/*
 * OS-dependent functions for Windows.
 * Implements osdep_get_name(), osdep_get_cwd(), osdep_event_init().
 */

#ifdef _WIN32

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "tmux.h"

#include <windows.h>
#include <psapi.h>

/*
 * Get the name of the foreground process in a pane.
 * Uses QueryFullProcessImageName to get the executable path,
 * then extracts the basename.
 */
char *
osdep_get_name(int fd, char *tty)
{
	HANDLE h;
	DWORD pid;
	char path[MAX_PATH];
	DWORD size = sizeof path;
	char *name, *slash;

	(void)fd;
	(void)tty;

	/*
	 * We need the PID. The fd is a socket bridged from ConPTY,
	 * so we can't easily get the foreground PID from it.
	 * Return NULL and let tmux use the stored command name.
	 */
	return (NULL);
}

/*
 * Get the current working directory of a process.
 * On Windows, this requires reading the PEB which is complex.
 * Return NULL and let tmux use the stored CWD.
 */
char *
osdep_get_cwd(int fd)
{
	(void)fd;
	return (NULL);
}

/*
 * Initialize the event system.
 */
struct event_base *
osdep_event_init(void)
{
	return (event_init());
}

#endif /* _WIN32 */
