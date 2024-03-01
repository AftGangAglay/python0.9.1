/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Tokenizer implementation */

/* XXX This is rather old, should be restructured perhaps */

/* XXX Need a better interface to report errors than writing to stderr */

/* XXX Should use editor resource to fetch true tab size on Macintosh */

#include <python/std.h>
#include <python/fgetsintr.h>
#include <python/tokenizer.h>
#include <python/result.h>
#include <python/token.h>

#ifndef TABSIZE
#define TABSIZE 8
#endif

/* Forward */
static struct py_tokenizer* tok_new(void);

static int tok_nextc(struct py_tokenizer* tok);

static void tok_backup(struct py_tokenizer* tok, int c);

/* Token names */

char* py_token_names[] = {
		"PY_ENDMARKER", "PY_NAME", "PY_NUMBER", "PY_STRING", "PY_NEWLINE", "PY_INDENT", "PY_DEDENT",
		"PY_LPAR", "PY_RPAR", "PY_LSQB", "PY_RSQB", "PY_COLON", "PY_COMMA", "PY_SEMI", "PY_PLUS",
		"PY_MINUS", "PY_STAR", "PY_SLASH", "PY_VBAR", "PY_AMPER", "PY_LESS", "PY_GREATER", "PY_EQUAL",
		"PY_DOT", "PY_PERCENT", "PY_BACKQUOTE", "PY_LBRACE", "PY_RBRACE", "PY_OP", "<PY_ERRORTOKEN>",
		"<PY_N_TOKENS>" };


/* Create and initialize a new tok_state structure */

static struct py_tokenizer* tok_new() {
	struct py_tokenizer* tok = malloc(sizeof(struct py_tokenizer));
	if(tok == NULL) {
		return NULL;
	}
	tok->buf = tok->cur = tok->end = tok->inp = NULL;
	tok->done = PY_RESULT_OK;
	tok->fp = NULL;
	tok->tabsize = TABSIZE;
	tok->indent = 0;
	tok->indstack[0] = 0;
	tok->atbol = 1;
	tok->pendin = 0;
	tok->prompt = tok->nextprompt = NULL;
	tok->lineno = 0;
	return tok;
}

/* Set up tokenizer for file */

struct py_tokenizer* py_tokenizer_setup_file(fp, ps1, ps2)FILE* fp;
										  char* ps1, * ps2;
{
	struct py_tokenizer* tok = tok_new();
	if(tok == NULL) {
		return NULL;
	}
	if((tok->buf = malloc(BUFSIZ * sizeof(char))) == NULL) {
		free(tok);
		return NULL;
	}
	tok->cur = tok->inp = tok->buf;
	tok->end = tok->buf + BUFSIZ;
	tok->fp = fp;
	tok->prompt = ps1;
	tok->nextprompt = ps2;
	return tok;
}


/* Free a tok_state structure */

void py_tokenizer_delete(tok)struct py_tokenizer* tok;
{
	/* XXX really need a separate flag to say 'my buffer' */
	if(tok->fp != NULL && tok->buf != NULL) {
		free(tok->buf);
	}
	free(tok);
}


/* Get next char, updating state; error code goes into tok->done */

static int tok_nextc(tok)struct py_tokenizer* tok;
{
	if(tok->done != PY_RESULT_OK) {
		return EOF;
	}

	for(;;) {
		if(tok->cur < tok->inp) {
			return *tok->cur++;
		}
		if(tok->fp == NULL) {
			tok->done = PY_RESULT_EOF;
			return EOF;
		}
		if(tok->inp > tok->buf && tok->inp[-1] == '\n') {
			tok->inp = tok->buf;
		}
		if(tok->inp == tok->end) {
			int n = tok->end - tok->buf;
			char* new = tok->buf;
			/* TODO: Leaky realloc. */
			new = realloc(new, (n + n) * sizeof(char));
			if(new == NULL) {
				fprintf(stderr, "tokenizer out of mem\n");
				tok->done = PY_RESULT_OOM;
				return EOF;
			}
			tok->buf = new;
			tok->inp = tok->buf + n;
			tok->end = tok->inp + n;
		}
		{
			tok->cur = tok->inp;
			if(tok->prompt != NULL && tok->inp == tok->buf) {
				fprintf(stderr, "%s", tok->prompt);
				tok->prompt = tok->nextprompt;
			}
			tok->done = py_fgets_intr(
					tok->inp, (int) (tok->end - tok->inp), tok->fp);
		}
		if(tok->done != PY_RESULT_OK) {
			if(tok->prompt != NULL) {
				fprintf(stderr, "\n");
			}
			return EOF;
		}
		tok->inp = strchr(tok->inp, '\0');
	}
}


/* Back-up one character */

static void tok_backup(tok, c)struct py_tokenizer* tok;
							  int c;
{
	if(c != EOF && c != 0xFF) {
		if(--tok->cur < tok->buf) {
			fprintf(stderr, "tok_backup: begin of buffer\n");
			abort();
		}
		if(*tok->cur != c) {
			*tok->cur = c;
		}
	}
}


/* Return the token corresponding to a single character */

int py_token_char(c)int c;
{
	switch(c) {
		case '(': return PY_LPAR;
		case ')': return PY_RPAR;
		case '[': return PY_LSQB;
		case ']': return PY_RSQB;
		case ':': return PY_COLON;
		case ',': return PY_COMMA;
		case ';': return PY_SEMI;
		case '+': return PY_PLUS;
		case '-': return PY_MINUS;
		case '*': return PY_STAR;
		case '/': return PY_SLASH;
		case '|': return PY_VBAR;
		case '&': return PY_AMPER;
		case '<': return PY_LESS;
		case '>': return PY_GREATER;
		case '=': return PY_EQUAL;
		case '.': return PY_DOT;
		case '%': return PY_PERCENT;
		case '`': return PY_BACKQUOTE;
		case '{': return PY_LBRACE;
		case '}': return PY_RBRACE;
		default: return PY_OP;
	}
}


/* Get next token, after space stripping etc. */

int py_tokenizer_get(tok, p_start, p_end)
		struct py_tokenizer* tok; /* In/out: tokenizer state */
		char** p_start, ** p_end; /* Out: point to start/end of token */
{
	int c;

	/* Get indentation level */
	if(tok->atbol) {
		int col = 0;
		tok->atbol = 0;
		tok->lineno++;
		for(;;) {
			c = tok_nextc(tok);
			if(c == ' ') {
				col++;
			}
			else if(c == '\t') {
				col = (col / tok->tabsize + 1) * tok->tabsize;
			}
			else {
				break;
			}
		}
		tok_backup(tok, c);
		if(col == tok->indstack[tok->indent]) {
			/* No change */
		}
		else if(col > tok->indstack[tok->indent]) {
			/* Indent -- always one */
			if(tok->indent + 1 >= PY_MAX_INDENT) {
				fprintf(stderr, "excessive indent\n");
				tok->done = PY_RESULT_TOKEN;
				return PY_ERRORTOKEN;
			}
			tok->pendin++;
			tok->indstack[++tok->indent] = col;
		}
		else /* col < tok->indstack[tok->indent] */ {
			/* Dedent -- any number, must be consistent */
			while(tok->indent > 0 && col < tok->indstack[tok->indent]) {
				tok->indent--;
				tok->pendin--;
			}
			if(col != tok->indstack[tok->indent]) {
				fprintf(stderr, "inconsistent dedent\n");
				tok->done = PY_RESULT_TOKEN;
				return PY_ERRORTOKEN;
			}
		}
	}

	*p_start = *p_end = tok->cur;

	/* Return pending indents/dedents */
	if(tok->pendin != 0) {
		if(tok->pendin < 0) {
			tok->pendin++;
			return PY_DEDENT;
		}
		else {
			tok->pendin--;
			return PY_INDENT;
		}
	}

	/*
	 * NOTE(agapatch): This is a bit of a janky way to fix files following
	 * 				  Scripts in packs. This may make unexpected EOF cause
	 * 				  Crashes or unstable script engine state.
	 */
	if(c == EOF || c == 0xFF) {
		return PY_ENDMARKER;
	}

	again:
	/* Skip spaces */
	do {
		c = tok_nextc(tok);
	} while(c == ' ' || c == '\t' || c == '\r');

	/* Set start of current token */
	*p_start = tok->cur - 1;

	/* Skip comment */
	if(c == '#') {
		/* Hack to allow overriding the tabsize in the file.
		   This is also recognized by vi, when it occurs near the
		   beginning or end of the file. (Will vi never die...?) */
		int x;
		/* XXX The case to (unsigned char *) is needed by THINK C 3.0 */
		if(sscanf(/*(unsigned char *)*/tok->cur, " vi:set tabsize=%d:", &x) ==
		   1 && x >= 1 && x <= 40) {
			fprintf(stderr, "# vi:set tabsize=%d:\n", x);
			tok->tabsize = x;
		}
		do {
			c = tok_nextc(tok);
		} while(c != EOF && c != 0xFF && c != '\n');
	}

	/* Check for EOF and errors now */
	if(c == EOF || c == 0xFF) {
		return tok->done == PY_RESULT_EOF ? PY_ENDMARKER : PY_ERRORTOKEN;
	}

	/* Identifier (most frequent token!) */
	if(isalpha(c) || c == '_') {
		do {
			c = tok_nextc(tok);
		} while(isalnum(c) || c == '_');
		tok_backup(tok, c);
		*p_end = tok->cur;
		return PY_NAME;
	}

	/* Newline */
	if(c == '\n') {
		tok->atbol = 1;
		*p_end = tok->cur - 1; /* Leave '\n' out of the string */
		return PY_NEWLINE;
	}

	/* Number */
	if(isdigit(c)) {
		if(c == '0') {
			/* Hex or octal */
			c = tok_nextc(tok);
			if(c == '.') {
				goto fraction;
			}
			if(c == 'x' || c == 'X') {
				/* Hex */
				do {
					c = tok_nextc(tok);
				} while(isxdigit(c));
			}
			else {
				/* Octal; c is first char of it */
				/* There's no 'isoctdigit' macro, sigh */
				while('0' <= c && c < '8') {
					c = tok_nextc(tok);
				}
			}
		}
		else {
			/* Decimal */
			do {
				c = tok_nextc(tok);
			} while(isdigit(c));
			/* Accept floating point numbers.
			   XXX This accepts incomplete things like 12e or 1e+;
				   worry about that at run-time.
			   XXX Doesn't accept numbers starting with a dot */
			if(c == '.') {
				fraction:
				/* Fraction */
				do {
					c = tok_nextc(tok);
				} while(isdigit(c));
			}
			if(c == 'e' || c == 'E') {
				/* Exponent part */
				c = tok_nextc(tok);
				if(c == '+' || c == '-') {
					c = tok_nextc(tok);
				}
				while(isdigit(c)) {
					c = tok_nextc(tok);
				}
			}
		}
		tok_backup(tok, c);
		*p_end = tok->cur;
		return PY_NUMBER;
	}

	/* String */
	if(c == '\'') {
		for(;;) {
			c = tok_nextc(tok);
			if(c == '\n' || c == EOF || c == 0xFF) {
				tok->done = PY_RESULT_TOKEN;
				return PY_ERRORTOKEN;
			}
			if(c == '\\') {
				c = tok_nextc(tok);
				*p_end = tok->cur;
				if(c == '\n' || c == EOF || c == 0xFF) {
					tok->done = PY_RESULT_TOKEN;
					return PY_ERRORTOKEN;
				}
				continue;
			}
			if(c == '\'') {
				break;
			}
		}
		*p_end = tok->cur;
		return PY_STRING;
	}

	/* Line continuation */
	if(c == '\\') {
		c = tok_nextc(tok);
		if(c == '\r') c = tok_nextc(tok);
		if(c != '\n') {
			tok->done = PY_RESULT_TOKEN;
			return PY_ERRORTOKEN;
		}
		tok->lineno++;
		goto again; /* Read next line */
	}

	/* Punctuation character */
	*p_end = tok->cur;
	return py_token_char(c);
}


#ifdef _DEBUG

void tok_dump(type, start, end)int type;
							   char* start, * end;
{
	printf("%s", py_token_names[type]);
	if(type == PY_NAME || type == PY_NUMBER || type == PY_STRING || type == PY_OP) {
		printf("(%.*s)", (int) (end - start), start);
	}
}

#endif
