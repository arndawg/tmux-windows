/*
 * Minimal POSIX regex wrapper for Windows.
 * Uses PCRE2 if available, or a simple fallback.
 * For the initial port, we provide a basic implementation
 * using Windows API or a bundled regex engine.
 */

#ifndef WIN32_REGEX_H
#define WIN32_REGEX_H

#include <sys/types.h>

/* regex flags */
#define REG_EXTENDED  1
#define REG_ICASE     2
#define REG_NOSUB     4
#define REG_NEWLINE   8

/* regexec flags */
#define REG_NOTBOL    1
#define REG_NOTEOL    2

/* Error codes */
#define REG_NOMATCH   1
#define REG_BADPAT    2
#define REG_ESPACE    12

/* Maximum number of subexpressions. */
#define REG_MAXSUB    32

typedef struct {
	size_t  re_nsub;   /* number of parenthesized subexpressions */
	void   *re_pcre;   /* compiled pattern (opaque) */
	int     re_flags;
	char   *re_pattern; /* saved pattern string */
} regex_t;

typedef struct {
	int rm_so; /* start offset */
	int rm_eo; /* end offset */
} regmatch_t;

/* Defined in win32-regex.c */
int  regcomp(regex_t *preg, const char *pattern, int cflags);
int  regexec(const regex_t *preg, const char *string, size_t nmatch,
         regmatch_t pmatch[], int eflags);
void regfree(regex_t *preg);
size_t regerror(int errcode, const regex_t *preg, char *errbuf,
         size_t errbuf_size);

#endif /* WIN32_REGEX_H */
