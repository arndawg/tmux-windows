/*
 * Hardcoded xterm-256color terminal capabilities for Windows.
 * Replaces ncurses/terminfo dependency.
 * Windows Terminal and modern conhost support full VT sequences.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32-platform.h"

static const struct win32_terminfo_entry win32_terminfo[] = {
	/* Boolean capabilities */
	{ "am",    NULL, 0, 1, 3 },  /* auto_right_margin */
	{ "bce",   NULL, 0, 1, 3 },  /* back_color_erase */
	{ "AX",    NULL, 0, 1, 3 },  /* AX (default colors) */
	{ "hs",    NULL, 0, 0, 3 },  /* has_status_line */
	{ "mir",   NULL, 0, 1, 3 },  /* move_insert_mode */
	{ "xenl",  NULL, 0, 1, 3 },  /* eat_newline_glitch */

	/* Numeric capabilities */
	{ "colors", NULL, 256, 0, 2 },
	{ "cols",   NULL, 80,  0, 2 },
	{ "lines",  NULL, 24,  0, 2 },
	{ "pairs",  NULL, 32767, 0, 2 },

	/* String capabilities - cursor movement */
	{ "cup",   "\033[%i%p1%d;%p2%dH", 0, 0, 1 },  /* cursor_address */
	{ "cub1",  "\010",                 0, 0, 1 },  /* cursor_left */
	{ "cub",   "\033[%p1%dD",          0, 0, 1 },  /* parm_left_cursor */
	{ "cud1",  "\012",                 0, 0, 1 },  /* cursor_down */
	{ "cud",   "\033[%p1%dB",          0, 0, 1 },  /* parm_down_cursor */
	{ "cuf1",  "\033[C",               0, 0, 1 },  /* cursor_right */
	{ "cuf",   "\033[%p1%dC",          0, 0, 1 },  /* parm_right_cursor */
	{ "cuu1",  "\033[A",               0, 0, 1 },  /* cursor_up */
	{ "cuu",   "\033[%p1%dA",          0, 0, 1 },  /* parm_up_cursor */
	{ "home",  "\033[H",               0, 0, 1 },  /* cursor_home */
	{ "hpa",   "\033[%i%p1%dG",        0, 0, 1 },  /* column_address */

	/* String capabilities - scrolling */
	{ "csr",   "\033[%i%p1%d;%p2%dr",  0, 0, 1 },  /* change_scroll_region */
	{ "indn",  "\033[%p1%dS",          0, 0, 1 },  /* parm_index */
	{ "ind",   "\012",                 0, 0, 1 },  /* scroll_forward */
	{ "ri",    "\033M",                0, 0, 1 },  /* scroll_reverse */
	{ "rin",   "\033[%p1%dT",          0, 0, 1 },  /* parm_rindex */

	/* String capabilities - editing */
	{ "clear", "\033[H\033[2J",        0, 0, 1 },  /* clear_screen */
	{ "ed",    "\033[J",               0, 0, 1 },  /* clr_eos */
	{ "el",    "\033[K",               0, 0, 1 },  /* clr_eol */
	{ "el1",   "\033[1K",              0, 0, 1 },  /* clr_bol */
	{ "ech",   "\033[%p1%dX",          0, 0, 1 },  /* erase_chars */
	{ "dch1",  "\033[P",               0, 0, 1 },  /* delete_character */
	{ "dch",   "\033[%p1%dP",          0, 0, 1 },  /* parm_dch */
	{ "dl1",   "\033[M",               0, 0, 1 },  /* delete_line */
	{ "dl",    "\033[%p1%dM",          0, 0, 1 },  /* parm_delete_line */
	{ "ich",   "\033[%p1%d@",          0, 0, 1 },  /* parm_ich */
	{ "il1",   "\033[L",               0, 0, 1 },  /* insert_line */
	{ "il",    "\033[%p1%dL",          0, 0, 1 },  /* parm_insert_line */

	/* String capabilities - attributes */
	{ "bold",  "\033[1m",              0, 0, 1 },  /* enter_bold_mode */
	{ "dim",   "\033[2m",              0, 0, 1 },  /* enter_dim_mode */
	{ "sitm",  "\033[3m",              0, 0, 1 },  /* enter_italics_mode */
	{ "smul",  "\033[4m",              0, 0, 1 },  /* enter_underline_mode */
	{ "blink", "\033[5m",              0, 0, 1 },  /* enter_blink_mode */
	{ "rev",   "\033[7m",              0, 0, 1 },  /* enter_reverse_mode */
	{ "invis", "\033[8m",              0, 0, 1 },  /* enter_secure_mode */
	{ "smxx",  "\033[9m",              0, 0, 1 },  /* enter_strikeout_mode */
	{ "sgr0",  "\033(B\033[m",         0, 0, 1 },  /* exit_attribute_mode */
	{ "ritm",  "\033[23m",             0, 0, 1 },  /* exit_italics_mode */
	{ "rmul",  "\033[24m",             0, 0, 1 },  /* exit_underline_mode */
	{ "rmxx",  "\033[29m",             0, 0, 1 },  /* exit_strikeout_mode */

	/* String capabilities - colors */
	{ "setaf",  "\033[%?%p1%{8}%<%t3%p1%d%e%p1%{16}%<%t9%p1%{8}%-%d%e38;5;%p1%d%;m",
	                                   0, 0, 1 },  /* set_a_foreground */
	{ "setab",  "\033[%?%p1%{8}%<%t4%p1%d%e%p1%{16}%<%t10%p1%{8}%-%d%e48;5;%p1%d%;m",
	                                   0, 0, 1 },  /* set_a_background */
	{ "op",    "\033[39;49m",          0, 0, 1 },  /* orig_pair */

	/* String capabilities - cursor visibility */
	{ "civis", "\033[?25l",            0, 0, 1 },  /* cursor_invisible */
	{ "cnorm", "\033[?12l\033[?25h",   0, 0, 1 },  /* cursor_normal */
	{ "cvvis", "\033[?12;25h",         0, 0, 1 },  /* cursor_visible */

	/* String capabilities - alternate screen */
	{ "smcup", "\033[?1049h\033[22;0;0t", 0, 0, 1 },  /* enter_ca_mode */
	{ "rmcup", "\033[?1049l\033[23;0;0t", 0, 0, 1 },  /* exit_ca_mode */

	/* String capabilities - keypad */
	{ "smkx",  "\033[?1h\033=",        0, 0, 1 },  /* keypad_xmit */
	{ "rmkx",  "\033[?1l\033>",        0, 0, 1 },  /* keypad_local */

	/* String capabilities - tabs */
	{ "ht",    "\011",                 0, 0, 1 },  /* tab */
	{ "tbc",   "\033[3g",              0, 0, 1 },  /* clear_all_tabs */
	{ "hts",   "\033H",               0, 0, 1 },  /* set_tab */

	/* String capabilities - bell */
	{ "bel",   "\007",                 0, 0, 1 },  /* bell */

	/* String capabilities - misc */
	{ "cr",    "\015",                 0, 0, 1 },  /* carriage_return */
	{ "E3",    "\033[3J",              0, 0, 1 },  /* clr_scrollback */
	{ "enacs", "",                     0, 0, 1 },  /* ena_acs */
	{ "smacs", "\033(0",               0, 0, 1 },  /* enter_alt_charset_mode */
	{ "rmacs", "\033(B",               0, 0, 1 },  /* exit_alt_charset_mode */

	/* ACS characters (box drawing). */
	{ "acsc",  "``aaffggiijjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~",
	                                   0, 0, 1 },

	/* Status line. */
	{ "tsl",   "\033]0;",             0, 0, 1 },  /* to_status_line */
	{ "fsl",   "\007",                0, 0, 1 },  /* from_status_line */

	/* Key codes */
	{ "kcub1", "\033OD",               0, 0, 1 },  /* key_left */
	{ "kcuf1", "\033OC",               0, 0, 1 },  /* key_right */
	{ "kcuu1", "\033OA",               0, 0, 1 },  /* key_up */
	{ "kcud1", "\033OB",               0, 0, 1 },  /* key_down */
	{ "kbs",   "\177",                 0, 0, 1 },  /* key_backspace */
	{ "kdch1", "\033[3~",              0, 0, 1 },  /* key_dc */
	{ "khome", "\033OH",               0, 0, 1 },  /* key_home */
	{ "kend",  "\033OF",               0, 0, 1 },  /* key_end */
	{ "kpp",   "\033[5~",              0, 0, 1 },  /* key_ppage */
	{ "knp",   "\033[6~",              0, 0, 1 },  /* key_npage */
	{ "kich1", "\033[2~",              0, 0, 1 },  /* key_ic */
	{ "kcbt",  "\033[Z",               0, 0, 1 },  /* key_btab */
	{ "kf1",   "\033OP",               0, 0, 1 },
	{ "kf2",   "\033OQ",               0, 0, 1 },
	{ "kf3",   "\033OR",               0, 0, 1 },
	{ "kf4",   "\033OS",               0, 0, 1 },
	{ "kf5",   "\033[15~",             0, 0, 1 },
	{ "kf6",   "\033[17~",             0, 0, 1 },
	{ "kf7",   "\033[18~",             0, 0, 1 },
	{ "kf8",   "\033[19~",             0, 0, 1 },
	{ "kf9",   "\033[20~",             0, 0, 1 },
	{ "kf10",  "\033[21~",             0, 0, 1 },
	{ "kf11",  "\033[23~",             0, 0, 1 },
	{ "kf12",  "\033[24~",             0, 0, 1 },

	/* Bracketed paste. */
	{ "Enbp",  "\033[?2004h",          0, 0, 1 },
	{ "Dsbp",  "\033[?2004l",          0, 0, 1 },

	/* Extended key sequences (modifiers). */
	{ "Eneks", "\033[>4;1m",           0, 0, 1 },
	{ "Dseks", "\033[>4;0m",           0, 0, 1 },

	/* Focus events. */
	{ "Enfcs", "\033[?1004h",          0, 0, 1 },
	{ "Dsfcs", "\033[?1004l",          0, 0, 1 },

	/* Cursor style (DECSCUSR). */
	{ "Cs",    "\033[%p1%d q",         0, 0, 1 },
	{ "Cr",    "\033[2 q",             0, 0, 1 },

	/* DECSLRM margins. */
	{ "Cmg",   "\033[%i%p1%d;%p2%ds",  0, 0, 1 },
	{ "Clmg",  "\033[s",               0, 0, 1 },
	{ "Enmg",  "\033[?69h",            0, 0, 1 },
	{ "Dsmg",  "\033[?69l",            0, 0, 1 },

	/* Hyperlinks. */
	{ "Hls",   "\033]8;%p1%s;%p2%s\033\\", 0, 0, 1 },

	{ NULL, NULL, 0, 0, 0 }
};

/*
 * Look up a terminal capability by name.
 * Returns the entry or NULL.
 */
const struct win32_terminfo_entry *
win32_terminfo_find(const char *name)
{
	const struct win32_terminfo_entry *e;

	for (e = win32_terminfo; e->name != NULL; e++) {
		if (strcmp(e->name, name) == 0)
			return (e);
	}
	return (NULL);
}

/*
 * Get count of terminfo entries (excluding sentinel).
 */
u_int
win32_terminfo_count(void)
{
	u_int n = 0;
	const struct win32_terminfo_entry *e;

	for (e = win32_terminfo; e->name != NULL; e++)
		n++;
	return (n);
}

/*
 * Get all terminfo entries and optionally the count.
 */
const struct win32_terminfo_entry *
win32_terminfo_table(u_int *count)
{
	if (count != NULL)
		*count = win32_terminfo_count();
	return (win32_terminfo);
}
