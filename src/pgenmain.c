/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Parser generator main program */

/*
 * This expects a grammar_file containing the grammar as argv[1]
 * It writes its output on two files in the current directory:
 * - "graminit.c" gets the grammar as a bunch of initialized data
 * - "graminit.h" gets the grammar's non-terminals as #defines.
 * Error messages and status info during the generation process are
 * written to stdout, or sometimes to stderr.
 */

#include <python/grammar.h>
#include <python/node.h>
#include <python/parsetok.h>
#include <python/pgen.h>

#include <asys/stream.h>
#include <asys/error.h>
#include <asys/main.h>

/* TODO: Leaky stream EH throughout this file. */

enum asys_result py_load_grammar(
		const char* grammar_file, struct py_grammar** grammar) {

	enum asys_result result;

	struct asys_stream stream;
	struct py_node* n = 0;

	if((result = asys_stream_new(&stream, grammar_file))) return result;

	py_parse_file(
			&stream, grammar_file, &py_meta_grammar, py_meta_grammar.start,
			&n);

	if(!n) return ASYS_RESULT_ERROR;

	if(!(*grammar = py_grammar_gen(n))) return ASYS_RESULT_ERROR;

	return ASYS_RESULT_OK;
}

enum asys_result asys_main(struct asys_main_data* main_data) {
	enum asys_result result;

	struct py_grammar* grammar;

	struct asys_stream stream;

	const char* grammar_file;
	const char* source_file;
	const char* header_file;

	if(main_data->argc != 4) {
		asys_result_fatal(
				__FILE__, "err: usage: pgenmain <grammar> <source> <header>",
				ASYS_RESULT_BAD_PARAM);
	}

	grammar_file = main_data->argv[1];
	source_file = main_data->argv[2];
	header_file = main_data->argv[3];

	if((result = py_load_grammar(grammar_file, &grammar))) return result;

	/* Write source. */
	{
		result = asys_stream_new_write(&stream, source_file);
		if(result) return result;

		if((result = py_grammar_print(grammar, &stream))) return result;

		if((result = asys_stream_delete(&stream))) return result;
	}

	/* Write header. */
	{
		result = asys_stream_new_write(&stream, header_file);
		if(result) return result;

		result = py_grammar_print_nonterminals(grammar, &stream);
		if(result) return result;

		if((result = asys_stream_delete(&stream))) return result;
	}

	return ASYS_RESULT_OK;
}

void py_fatal(const char* msg) {
	asys_result_fatal(__FILE__, msg, ASYS_RESULT_ERROR);
}
