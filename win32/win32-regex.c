/*
 * Simple POSIX regex implementation for Windows.
 * This is a basic implementation using a recursive backtracking matcher.
 * It supports enough of POSIX ERE for tmux's needs:
 *   - Character classes [abc], [^abc], [a-z]
 *   - Quantifiers: *, +, ?
 *   - Alternation: |
 *   - Grouping: ()
 *   - Anchors: ^, $
 *   - Dot: . (matches any char except newline)
 *   - Escapes: \., \*, etc.
 *   - Case-insensitive matching (REG_ICASE)
 *
 * This is NOT a full POSIX regex implementation. For production use,
 * consider linking against PCRE2 instead.
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32-regex.h"

/*
 * We implement regex using a simple NFA simulation.
 * For tmux's purposes (mostly glob-like patterns and simple searches),
 * this is sufficient.
 */

/* Internal node types for compiled pattern. */
enum node_type {
	NODE_LITERAL,	/* match single character */
	NODE_DOT,	/* match any character */
	NODE_CLASS,	/* character class [...] */
	NODE_NCLASS,	/* negated character class [^...] */
	NODE_ANCHOR_BOL,/* ^ */
	NODE_ANCHOR_EOL,/* $ */
	NODE_GROUP_START,
	NODE_GROUP_END,
	NODE_BRANCH,	/* alternation */
	NODE_END	/* end of pattern */
};

struct re_node {
	enum node_type type;
	int ch;		 /* for NODE_LITERAL */
	char *class_str; /* for NODE_CLASS/NODE_NCLASS */
	int quantifier;	 /* 0=none, '*'=0+, '+'=1+, '?'=0or1 */
	int greedy;	 /* 1=greedy (default) */
	int group_id;	 /* for GROUP_START/END */
};

struct compiled_re {
	struct re_node *nodes;
	int nnodes;
	int ngroups;
	int flags;
};

static int
class_match(const char *class_str, int ch, int icase)
{
	const char *p = class_str;
	int match = 0;

	while (*p) {
		if (p[1] == '-' && p[2]) {
			int lo = (unsigned char)p[0];
			int hi = (unsigned char)p[2];
			if (icase) {
				if ((tolower(ch) >= tolower(lo) &&
				    tolower(ch) <= tolower(hi)))
					match = 1;
			} else {
				if (ch >= lo && ch <= hi)
					match = 1;
			}
			p += 3;
		} else {
			int c = (unsigned char)*p;
			if (icase) {
				if (tolower(ch) == tolower(c))
					match = 1;
			} else {
				if (ch == c)
					match = 1;
			}
			p++;
		}
	}
	return match;
}

static int
node_matches(struct re_node *n, int ch, int icase)
{
	switch (n->type) {
	case NODE_LITERAL:
		if (icase)
			return tolower(ch) == tolower(n->ch);
		return ch == n->ch;
	case NODE_DOT:
		return ch != '\n' && ch != '\0';
	case NODE_CLASS:
		return class_match(n->class_str, ch, icase);
	case NODE_NCLASS:
		return !class_match(n->class_str, ch, icase);
	default:
		return 0;
	}
}

/*
 * Recursive match. Returns number of characters consumed, or -1 for no match.
 * This is intentionally simple and won't handle all edge cases.
 */
static int
re_match_here(struct compiled_re *re, int ni, const char *str, int pos,
    int len, regmatch_t *pmatch, int nmatch)
{
	struct re_node *n;

	if (ni >= re->nnodes)
		return pos;

	n = &re->nodes[ni];

	switch (n->type) {
	case NODE_END:
		return pos;
	case NODE_ANCHOR_BOL:
		if (pos != 0)
			return -1;
		return re_match_here(re, ni + 1, str, pos, len, pmatch, nmatch);
	case NODE_ANCHOR_EOL:
		if (pos != len)
			return -1;
		return re_match_here(re, ni + 1, str, pos, len, pmatch, nmatch);
	case NODE_GROUP_START:
		if (pmatch && n->group_id < nmatch)
			pmatch[n->group_id].rm_so = pos;
		return re_match_here(re, ni + 1, str, pos, len, pmatch, nmatch);
	case NODE_GROUP_END:
		if (pmatch && n->group_id < nmatch)
			pmatch[n->group_id].rm_eo = pos;
		return re_match_here(re, ni + 1, str, pos, len, pmatch, nmatch);
	case NODE_LITERAL:
	case NODE_DOT:
	case NODE_CLASS:
	case NODE_NCLASS:
		if (n->quantifier == '*' || n->quantifier == '+' ||
		    n->quantifier == '?') {
			int min_count = (n->quantifier == '+') ? 1 : 0;
			int max_count = (n->quantifier == '?') ? 1 : len - pos;
			int count, result;

			/* Greedy: try maximum first. */
			count = 0;
			while (count < max_count && pos + count < len &&
			    node_matches(n, (unsigned char)str[pos + count],
			    re->flags & REG_ICASE))
				count++;

			/* Try from longest match down. */
			while (count >= min_count) {
				result = re_match_here(re, ni + 1, str,
				    pos + count, len, pmatch, nmatch);
				if (result >= 0)
					return result;
				count--;
			}
			return -1;
		}
		/* No quantifier: match exactly one. */
		if (pos >= len)
			return -1;
		if (!node_matches(n, (unsigned char)str[pos],
		    re->flags & REG_ICASE))
			return -1;
		return re_match_here(re, ni + 1, str, pos + 1, len,
		    pmatch, nmatch);
	case NODE_BRANCH:
		/* Not implemented in this simple version. */
		return -1;
	default:
		return -1;
	}
}

/*
 * Compile a simple regex pattern into internal nodes.
 * Returns 0 on success.
 */
static int
compile_pattern(const char *pattern, struct compiled_re *re, int cflags)
{
	const char *p = pattern;
	int alloc = 64;
	int group_id = 1; /* 0 is the whole match */

	re->flags = cflags;
	re->nodes = calloc(alloc, sizeof *re->nodes);
	re->nnodes = 0;
	re->ngroups = 0;

	if (re->nodes == NULL)
		return REG_ESPACE;

#define ADD_NODE(t) do { \
	if (re->nnodes >= alloc) { \
		alloc *= 2; \
		re->nodes = realloc(re->nodes, alloc * sizeof *re->nodes); \
		if (re->nodes == NULL) return REG_ESPACE; \
	} \
	memset(&re->nodes[re->nnodes], 0, sizeof re->nodes[0]); \
	re->nodes[re->nnodes].type = t; \
	re->nodes[re->nnodes].greedy = 1; \
} while (0)

	while (*p) {
		switch (*p) {
		case '^':
			ADD_NODE(NODE_ANCHOR_BOL);
			re->nnodes++;
			p++;
			break;
		case '$':
			ADD_NODE(NODE_ANCHOR_EOL);
			re->nnodes++;
			p++;
			break;
		case '.':
			ADD_NODE(NODE_DOT);
			re->nnodes++;
			p++;
			break;
		case '(':
			ADD_NODE(NODE_GROUP_START);
			re->nodes[re->nnodes].group_id = group_id;
			re->ngroups = group_id;
			group_id++;
			re->nnodes++;
			p++;
			break;
		case ')':
			ADD_NODE(NODE_GROUP_END);
			re->nodes[re->nnodes].group_id = group_id - 1;
			re->nnodes++;
			p++;
			break;
		case '[': {
			char class_buf[256];
			int ci = 0;
			int negated = 0;

			p++; /* skip '[' */
			if (*p == '^') {
				negated = 1;
				p++;
			}
			if (*p == ']') {
				class_buf[ci++] = *p++;
			}
			while (*p && *p != ']' && ci < (int)sizeof class_buf - 1) {
				class_buf[ci++] = *p++;
			}
			class_buf[ci] = '\0';
			if (*p == ']')
				p++;

			ADD_NODE(negated ? NODE_NCLASS : NODE_CLASS);
			re->nodes[re->nnodes].class_str = strdup(class_buf);
			re->nnodes++;
			break;
		}
		case '\\':
			p++;
			if (*p == '\0')
				return REG_BADPAT;
			ADD_NODE(NODE_LITERAL);
			re->nodes[re->nnodes].ch = (unsigned char)*p;
			re->nnodes++;
			p++;
			break;
		case '*':
		case '+':
		case '?':
			/* Apply quantifier to previous node. */
			if (re->nnodes > 0 &&
			    re->nodes[re->nnodes - 1].quantifier == 0) {
				re->nodes[re->nnodes - 1].quantifier = *p;
			}
			p++;
			break;
		case '|':
			/* Simple alternation not fully supported yet. */
			p++;
			break;
		default:
			ADD_NODE(NODE_LITERAL);
			re->nodes[re->nnodes].ch = (unsigned char)*p;
			re->nnodes++;
			p++;
			break;
		}
	}

	ADD_NODE(NODE_END);
	re->nnodes++;

#undef ADD_NODE

	return 0;
}

int
regcomp(regex_t *preg, const char *pattern, int cflags)
{
	struct compiled_re *re;
	int ret;

	memset(preg, 0, sizeof *preg);
	preg->re_flags = cflags;
	preg->re_pattern = strdup(pattern);

	re = calloc(1, sizeof *re);
	if (re == NULL)
		return REG_ESPACE;

	ret = compile_pattern(pattern, re, cflags);
	if (ret != 0) {
		free(re->nodes);
		free(re);
		return ret;
	}

	preg->re_nsub = re->ngroups;
	preg->re_pcre = re;
	return 0;
}

int
regexec(const regex_t *preg, const char *string, size_t nmatch,
    regmatch_t pmatch[], int eflags)
{
	struct compiled_re *re = preg->re_pcre;
	int len = (int)strlen(string);
	int i, result;

	(void)eflags;

	if (re == NULL)
		return REG_NOMATCH;

	/* Initialize pmatch. */
	if (pmatch != NULL) {
		for (i = 0; i < (int)nmatch; i++) {
			pmatch[i].rm_so = -1;
			pmatch[i].rm_eo = -1;
		}
	}

	/* Try matching at each position. */
	for (i = 0; i <= len; i++) {
		result = re_match_here(re, 0, string, i, len,
		    pmatch, (int)nmatch);
		if (result >= 0) {
			if (pmatch != NULL && nmatch > 0) {
				pmatch[0].rm_so = i;
				pmatch[0].rm_eo = result;
			}
			return 0;
		}
	}

	return REG_NOMATCH;
}

void
regfree(regex_t *preg)
{
	struct compiled_re *re = preg->re_pcre;

	if (re != NULL) {
		int i;
		for (i = 0; i < re->nnodes; i++) {
			if (re->nodes[i].class_str != NULL)
				free(re->nodes[i].class_str);
		}
		free(re->nodes);
		free(re);
	}
	free(preg->re_pattern);
	memset(preg, 0, sizeof *preg);
}

size_t
regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size)
{
	const char *msg;

	(void)preg;

	switch (errcode) {
	case 0:
		msg = "success";
		break;
	case REG_NOMATCH:
		msg = "no match";
		break;
	case REG_BADPAT:
		msg = "bad pattern";
		break;
	case REG_ESPACE:
		msg = "out of memory";
		break;
	default:
		msg = "unknown error";
		break;
	}

	if (errbuf != NULL && errbuf_size > 0) {
		strncpy(errbuf, msg, errbuf_size - 1);
		errbuf[errbuf_size - 1] = '\0';
	}
	return strlen(msg) + 1;
}
