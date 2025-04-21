/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Tokenizer implementation */

/* TODO: Need a better interface to report errors than writing to stderr */

#include <python/tokenizer.h>
#include <python/result.h>
#include <python/token.h>
#include <python/errors.h>

#include <asys/stream.h>
#include <asys/memory.h>
#include <asys/string.h>
#include <asys/log.h>

#ifndef PY_TABSIZE
# define PY_TABSIZE (8)
#endif

/* Token names */

char* py_token_names[] = {
		"PY_ENDMARKER", "PY_NAME", "PY_NUMBER", "PY_STRING", "PY_NEWLINE",
		"PY_INDENT", "PY_DEDENT", "PY_LPAR", "PY_RPAR", "PY_LSQB", "PY_RSQB",
		"PY_COLON", "PY_COMMA", "PY_SEMI", "PY_PLUS", "PY_MINUS", "PY_STAR",
		"PY_SLASH", "PY_VBAR", "PY_AMPER", "PY_LESS", "PY_GREATER", "PY_EQUAL",
		"PY_DOT", "PY_PERCENT", "PY_LBRACE", "PY_RBRACE",
		"PY_OP", "<PY_ERRORTOKEN>", "<PY_N_TOKENS>" };


/* Create and initialize a new tok_state structure */

static struct py_tokenizer* py_tokenizer_new(void) {
	struct py_tokenizer* tokenizer;

	tokenizer = asys_memory_allocate_zero(1, sizeof(struct py_tokenizer));
	if(!tokenizer) return 0;

	tokenizer->done = PY_RESULT_OK;

	return tokenizer;
}

/* Set up tokenizer for file */

struct py_tokenizer* py_tokenizer_setup_file(struct asys_stream* fp) {
	struct py_tokenizer* tokenizer = py_tokenizer_new();
	if(!tokenizer) return 0;

	tokenizer->buf = asys_memory_allocate(
			ASYS_FIXED_BUFFER_SIZE * sizeof(char));

	if(!tokenizer->buf) {
		py_tokenizer_delete(tokenizer);
		return 0;
	}

	tokenizer->cur = tokenizer->inp = tokenizer->buf;
	tokenizer->end = tokenizer->buf + ASYS_FIXED_BUFFER_SIZE;
	tokenizer->fp = fp;

	return tokenizer;
}


/* Free a tok_state structure */

void py_tokenizer_delete(struct py_tokenizer* tokenizer) {
	/* TODO: really need a separate flag to say 'my buffer' */
	if(tokenizer->fp && tokenizer->buf) asys_memory_free(tokenizer->buf);

	asys_memory_free(tokenizer);
}


/* Get next char, updating state; error code goes into tokenizer->done */

static int py_tokenizer_next_character(struct py_tokenizer* tokenizer) {
	if(tokenizer->done != PY_RESULT_OK) return -1;

	for(;;) {
		asys_size_t size;

		if(tokenizer->cur < tokenizer->inp) return *tokenizer->cur++;

		if(!tokenizer->fp) {
			tokenizer->done = PY_RESULT_EOF;
			return -1;
		}

		if(tokenizer->inp > tokenizer->buf && tokenizer->inp[-1] == '\n') {
			tokenizer->inp = tokenizer->buf;
		}

		if(tokenizer->inp == tokenizer->end) {
			unsigned n = (unsigned) (tokenizer->end - tokenizer->buf);
			char* new = tokenizer->buf;

			new = asys_memory_reallocate_safe(new, 2 * n * sizeof(char));
			if(!new) {
				tokenizer->done = PY_RESULT_OOM;
				return -1;
			}

			tokenizer->buf = new;
			tokenizer->inp = tokenizer->buf + n;
			tokenizer->end = tokenizer->inp + n;
		}

		tokenizer->cur = tokenizer->inp;

		size = (asys_size_t) (tokenizer->end - tokenizer->inp);

		/* TODO: Better EH. */
		if(asys_stream_read_line(tokenizer->fp, tokenizer->inp, size)) {
			tokenizer->done = PY_RESULT_ERROR;
			return -1;
		}

		if(tokenizer->done != PY_RESULT_OK) return -1;

		tokenizer->inp = asys_string_find(tokenizer->inp, '\0');
	}
}


/* Back-up one character */

static void py_tokenizer_back(struct py_tokenizer* tokenizer, int c) {
	if(c != -1) {
		if(--tokenizer->cur < tokenizer->buf) {
			/* TODO: Better EH. */
			py_fatal("py_tokenizer_back: begin of buffer");
		}

		if(*tokenizer->cur != c) *tokenizer->cur = (char) c;
	}
}


/* Return the token corresponding to a single character */

int py_token_char(int c) {
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
		case '{': return PY_LBRACE;
		case '}': return PY_RBRACE;
		default: return PY_OP;
	}
}


/* Get next token, after space stripping etc. */

unsigned py_tokenizer_get(
		struct py_tokenizer* tokenizer, /* In/out: tokenizer state */
		char** p_start, /* Out: point to start/end of token */
		char** p_end) {

	int c = 0;

	/* Get indentation level */
	if(tokenizer->atbol) {
		int col = 0;

		tokenizer->atbol = 0;
		tokenizer->lineno++;

		for(;;) {
			c = py_tokenizer_next_character(tokenizer);

			if(c == ' ') col++;
			else if(c == '\t') col = (col / PY_TABSIZE + 1) * PY_TABSIZE;
			else break;
		}

		py_tokenizer_back(tokenizer, c);

		/* TODO: Reformat. */
		if(col == tokenizer->indstack[tokenizer->indent]) {
			/* No change */
		}
		else if(col > tokenizer->indstack[tokenizer->indent]) {
			/* Indent -- always one */
			if(tokenizer->indent + 1 >= PY_MAX_INDENT) {
				/* TODO: Better EH. */
				tokenizer->done = PY_RESULT_TOKEN;
				return PY_ERRORTOKEN;
			}

			tokenizer->pendin++;
			tokenizer->indstack[++tokenizer->indent] = col;
		}
		else /* col < tokenizer->indstack[tokenizer->indent] */ {
			/* Dedent -- any number, must be consistent */
			while(tokenizer->indent > 0 &&
					col < tokenizer->indstack[tokenizer->indent]) {

				tokenizer->indent--;
				tokenizer->pendin--;
			}

			if(col != tokenizer->indstack[tokenizer->indent]) {
				/* TODO: Better EH. */
				tokenizer->done = PY_RESULT_TOKEN;
				return PY_ERRORTOKEN;
			}
		}
	}

	*p_start = *p_end = tokenizer->cur;

	/* Return pending indents/dedents */
	if(tokenizer->pendin != 0) {
		if(tokenizer->pendin < 0) {
			tokenizer->pendin++;
			return PY_DEDENT;
		}
		else {
			tokenizer->pendin--;
			return PY_INDENT;
		}
	}

	/*
	 * NOTE(agapatch): This is a bit of a janky way to fix files following
	 * 				  Scripts in packs. This may make unexpected -1 cause
	 * 				  Crashes or unstable script engine state.
	 */
	if(c == -1 || c == 0xFF) return PY_ENDMARKER;

	again:
	/* Skip spaces */
	do {
		c = py_tokenizer_next_character(tokenizer);
	} while(c == ' ' || c == '\t' || c == '\r');

	/* Set start of current token */
	*p_start = tokenizer->cur - 1;

	/* Skip comment */
	if(c == '#') {
		do {
			c = py_tokenizer_next_character(tokenizer);
		} while(c != -1 && c != 0xFF && c != '\n');
	}

	/* Check for -1 and errors now */
	if(c == -1 || c == 0xFF) {
		return tokenizer->done == PY_RESULT_EOF ? PY_ENDMARKER : PY_ERRORTOKEN;
	}

	/* Identifier (most frequent token!) */
	if(asys_character_is_letter(c) || c == '_') {
		do {
			c = py_tokenizer_next_character(tokenizer);
		} while(asys_character_is_dec_digit(c) ||
				asys_character_is_letter(c) || c == '_');

		py_tokenizer_back(tokenizer, c);
		*p_end = tokenizer->cur;

		return PY_NAME;
	}

	/* Newline */
	if(c == '\n') {
		tokenizer->atbol = 1;
		*p_end = tokenizer->cur - 1; /* Leave '\n' out of the string */

		return PY_NEWLINE;
	}

	/* Number */
	if(asys_character_is_dec_digit(c)) {
		if(c == '0') {
			/* Hex or octal */
			c = py_tokenizer_next_character(tokenizer);
			if(c == '.') goto fraction;

			if(c == 'x' || c == 'X') {
				/* Hex */
				do {
					c = py_tokenizer_next_character(tokenizer);
				} while(asys_character_is_hex_digit(c));
			}
			else {
				/* Octal; c is first char of it */
				/* There's no 'isoctdigit' macro, sigh */
				while(asys_character_is_oct_digit(c)) {
					c = py_tokenizer_next_character(tokenizer);
				}
			}
		}
		else {
			/* Decimal */
			do {
				c = py_tokenizer_next_character(tokenizer);
			} while(asys_character_is_dec_digit(c));

			/* Accept floating point numbers.
			   XXX This accepts incomplete things like 12e or 1e+;
				   worry about that at run-time.
			   XXX Doesn't accept numbers starting with a dot */
			if(c == '.') {
				fraction:
				/* Fraction */
				do {
					c = py_tokenizer_next_character(tokenizer);
				} while(asys_character_is_dec_digit(c));
			}

			if(c == 'e' || c == 'E') {
				/* Exponent part */
				c = py_tokenizer_next_character(tokenizer);
				if(c == '+' || c == '-') {
					c = py_tokenizer_next_character(tokenizer);
				}

				while(asys_character_is_dec_digit(c)) {
					c = py_tokenizer_next_character(tokenizer);
				}
			}
		}

		py_tokenizer_back(tokenizer, c);
		*p_end = tokenizer->cur;

		return PY_NUMBER;
	}

	/* String */
	if(c == '\'') {
		for(;;) {
			c = py_tokenizer_next_character(tokenizer);
			if(c == '\n' || c == -1 || c == 0xFF) {
				tokenizer->done = PY_RESULT_TOKEN;
				return PY_ERRORTOKEN;
			}

			if(c == '\\') {
				c = py_tokenizer_next_character(tokenizer);
				*p_end = tokenizer->cur;

				if(c == '\n' || c == -1 || c == 0xFF) {
					tokenizer->done = PY_RESULT_TOKEN;
					return PY_ERRORTOKEN;
				}

				continue;
			}

			if(c == '\'') break;
		}

		*p_end = tokenizer->cur;

		return PY_STRING;
	}

	/* Line continuation */
	if(c == '\\') {
		c = py_tokenizer_next_character(tokenizer);
		if(c == '\r') c = py_tokenizer_next_character(tokenizer);
		if(c != '\n') {
			tokenizer->done = PY_RESULT_TOKEN;
			return PY_ERRORTOKEN;
		}
		tokenizer->lineno++;
		goto again; /* Read next line */
	}

	/* Punctuation character */
	*p_end = tokenizer->cur;
	return py_token_char(c);
}
