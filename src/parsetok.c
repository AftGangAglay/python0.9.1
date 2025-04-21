/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Parser-tokenizer link implementation */

#include <python/tokenizer.h>
#include <python/node.h>
#include <python/errors.h>
#include <python/grammar.h>
#include <python/parser.h>
#include <python/parsetok.h>
#include <python/result.h>
#include <python/token.h>

#include <asys/memory.h>
#include <asys/string.h>
#include <asys/log.h>

/* Parse input coming from the given tokenizer structure.
   Return error code. */

static int py_parse_token(
		struct py_tokenizer* tok, struct py_grammar* g, int start,
		struct py_node** n_ret) {

	struct py_parser* ps;
	int ret;

	if(!(ps = py_parser_new(g, start))) {
		/* TODO: Better EH. */
		py_fatal("oom");
	}

	for(;;) {
		char* a;
		char* b;
		unsigned type;
		unsigned len;
		char* str;

		asys_bool_t buf;

		type = py_tokenizer_get(tok, &a, &b);
		if(type == PY_ERRORTOKEN) {
			ret = tok->done;
			break;
		}

		len = (unsigned) (b - a);

		/*
		 * TODO: Do these need to be malloc'd? Can node tree strs just be refs
		 * 		 (or even just offsets?)
		 */
		str = asys_memory_allocate((len + 1) * sizeof(char));
		if(!str) {
			/* TODO: Better EH. */
			py_fatal("oom");
		}

		buf = a < tok->buf;
		/* TODO: Might be broken. */
		asys_memory_copy(str, buf ? tok->buf : a, len);
		str[len] = '\0';

		ret = py_parser_add(ps, type, str, tok->lineno);
		if(ret != PY_RESULT_OK) {
			if(ret == PY_RESULT_DONE) {
				*n_ret = ps->tree;
				ps->tree = 0;
			}
			else if(tok->lineno <= 1 && tok->done == PY_RESULT_EOF) {
				ret = PY_RESULT_EOF;
			}

			break;
		}
	}

	py_parser_delete(ps);
	return ret;
}

/* Parse input coming from a file. Return error code, print some errors. */

int py_parse_file(
		struct asys_stream* fp, const char* filename, struct py_grammar* g,
		int start, struct py_node** n_ret) {

	struct py_tokenizer* tok = py_tokenizer_setup_file(fp);
	int ret;

	if(!tok) {
		/* TODO: Better EH. */
		py_fatal("oom");
	}

	ret = py_parse_token(tok, g, start, n_ret);
	if(ret == PY_RESULT_TOKEN || ret == PY_RESULT_SYNTAX) {
		asys_log(
				__FILE__, "err: file %s, line %d:",
				filename, tok->lineno);

		*tok->inp = '\0';
		if(tok->inp > tok->buf && tok->inp[-1] == '\n') {
			tok->inp[-1] = '\0';
		}

		asys_log(__FILE__, "\t%s", tok->buf);

		/* TODO: Reimplement error column indicator. */
		/*
		for(p = tok->buf; p < tok->cur; p++) {
			if(*p == '\t') {
				putc('\t', stderr);
			}
			else {
				putc(' ', stderr);
			}
		}
		fprintf(stderr, "^\n");
		 */
	}

	py_tokenizer_delete(tok);

	return ret;
}
