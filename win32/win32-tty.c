/*
 * Windows console TTY handling.
 * Replaces termios raw mode with Windows Console API.
 * Enables VT processing for terminal escape sequences.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32-platform.h"

static HANDLE hConsoleIn = INVALID_HANDLE_VALUE;
static HANDLE hConsoleOut = INVALID_HANDLE_VALUE;
static DWORD saved_in_mode = 0;
static DWORD saved_out_mode = 0;
static int mode_saved = 0;

/*
 * Set console to raw VT mode.
 * Enables virtual terminal input and output processing.
 */
int
win32_tty_raw_mode(void)
{
	DWORD in_mode, out_mode;

	hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsoleIn == INVALID_HANDLE_VALUE ||
	    hConsoleOut == INVALID_HANDLE_VALUE)
		return (-1);

	/* Save current modes. */
	GetConsoleMode(hConsoleIn, &saved_in_mode);
	GetConsoleMode(hConsoleOut, &saved_out_mode);
	mode_saved = 1;

	/* Input: enable VT input, disable line input and echo. */
	in_mode = ENABLE_VIRTUAL_TERMINAL_INPUT;

	/* Output: enable VT processing and disable auto newlines. */
	out_mode = ENABLE_PROCESSED_OUTPUT |
	    ENABLE_VIRTUAL_TERMINAL_PROCESSING |
	    DISABLE_NEWLINE_AUTO_RETURN;

	SetConsoleMode(hConsoleIn, in_mode);
	SetConsoleMode(hConsoleOut, out_mode);

	return (0);
}

/*
 * Restore console mode.
 */
void
win32_tty_restore(void)
{
	if (!mode_saved)
		return;

	SetConsoleMode(hConsoleIn, saved_in_mode);
	SetConsoleMode(hConsoleOut, saved_out_mode);
	mode_saved = 0;
}

/*
 * Get console window size.
 */
int
win32_tty_get_size(int *cols, int *rows)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE h;

	h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h == INVALID_HANDLE_VALUE)
		return (-1);

	if (!GetConsoleScreenBufferInfo(h, &csbi))
		return (-1);

	*cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	*rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return (0);
}

/*
 * Initialize console for UTF-8.
 */
void
win32_tty_init_utf8(void)
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
}
