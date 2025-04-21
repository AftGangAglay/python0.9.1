/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Print arc bunch of C initializers that represent arc grammar */

#include <python/grammar.h>

#include <asys/stream.h>

static enum asys_result py_dfa_print_arcs(
		unsigned i, struct py_dfa* dfa, struct asys_stream* stream) {

	enum asys_result result;

	struct py_arc* arc;
	struct py_state* state = dfa->states;

	unsigned j, k;

	for(j = 0; j < dfa->count; j++, state++) {
		result = asys_stream_write_format(
				stream, "static struct py_arc arcs_%d_%d[%d] = {\n",
				i, j, state->count);

		if(result) return result;

		arc = state->arcs;

		for(k = 0; k < state->count; k++, arc++) {
			result = asys_stream_write_format(
					stream, "\t{ %d, %d },\n", arc->label, arc->arrow);

			if(result) return result;
		}

		result = asys_stream_write_format(stream, "};\n");
		if(result) return result;
	}

	return ASYS_RESULT_OK;
}

static enum asys_result py_grammar_print_states(
		struct py_grammar* grammar, struct asys_stream* stream) {

	enum asys_result result;

	struct py_state* state;
	struct py_dfa* dfa = grammar->dfas;

	unsigned i, j;

	for(i = 0; i < grammar->count; i++, dfa++) {
		if((result = py_dfa_print_arcs(i, dfa, stream))) return result;

		result = asys_stream_write_format(
				stream, "static struct py_state states_%d[%d] = {\n",
				i, dfa->count);

		if(result) return result;

		state = dfa->states;

		for(j = 0; j < dfa->count; j++, state++) {
			result = asys_stream_write_format(
					stream, "\t{ %d, arcs_%d_%d, 0, 0, 0, 0 },\n",
					state->count, i, j);

			if(result) return result;
		}

		result = asys_stream_write_format(stream, "};\n");
		if(result) return result;
	}

	return ASYS_RESULT_OK;
}

static enum asys_result py_grammar_print_dfas(
		struct py_grammar* grammar, struct asys_stream* stream) {

	enum asys_result result;

	struct py_dfa* dfa = grammar->dfas;

	unsigned i, j;

	if((result = py_grammar_print_states(grammar, stream))) return result;

	result = asys_stream_write_format(
			stream, "static struct py_dfa dfas[%d] = {\n", grammar->count);

	if(result) return result;

	for(i = 0; i < grammar->count; i++, dfa++) {
		result = asys_stream_write_format(
				stream, "\t{\n\t\t%d, \"%s\", %d, %d, states_%d,\n",
				dfa->type, dfa->name, dfa->initial, dfa->count, i);

		if(result) return result;

		result = asys_stream_write_format(stream, "\t (py_byte_t*) \"");
		if(result) return result;

		for(j = 0; j < PY_NBYTES(grammar->labels.count); j++) {
			result = asys_stream_write_format(
					stream, "\\%03o", dfa->first[j] & 0xFF);

			if(result) return result;
		}

		result = asys_stream_write_format(stream, "\"},\n");
		if(result) return result;
	}

	return asys_stream_write_format(stream, "};\n");
}

static enum asys_result py_grammar_print_labels(
		struct py_grammar* grammar, struct asys_stream* stream) {

	enum asys_result result;

	struct py_label* l = grammar->labels.label;

	unsigned i;

	result = asys_stream_write_format(
			stream, "static struct py_label labels[%d] = {\n",
			grammar->labels.count);

	if(result) return result;

	for(i = 0; i < grammar->labels.count; i++, l++) {
		if(!l->str) {
			result = asys_stream_write_format(
					stream, "\t{ %d, 0 },\n", l->type);

			if(result) return result;
		}
		else {
			result = asys_stream_write_format(
					stream, "\t{ %d, \"%s\" },\n", l->type, l->str);

			if(result) return result;
		}
	}

	return asys_stream_write_format(stream, "};\n");
}

enum asys_result py_grammar_print(
		struct py_grammar* grammar, struct asys_stream* stream) {

	enum asys_result result;

	result = asys_stream_write_format(stream, "#include <python/grammar.h>\n");
	if(result) return result;

	if((result = py_grammar_print_dfas(grammar, stream))) return result;
	if((result = py_grammar_print_labels(grammar, stream))) return result;

	return asys_stream_write_format(
			stream,
			"struct py_grammar py_grammar = {\n"
				"\t%d,\n"
				"\tdfas,\n"
				"\t{ %d, labels },\n"
				"\t%d,\n"
				"\t0\n"
			"};\n",
			grammar->count,
			grammar->labels.count,
			grammar->start);
}

enum asys_result py_grammar_print_nonterminals(
		struct py_grammar* grammar, struct asys_stream* stream) {

	enum asys_result result;

	struct py_dfa* dfa = grammar->dfas;

	unsigned i;

	for(i = 0; i < grammar->count; i++, dfa++) {
		result = asys_stream_write_format(
				stream, "#define %s (%d)\n", dfa->name, dfa->type);

		if(result) return result;
	}

	return ASYS_RESULT_OK;
}
