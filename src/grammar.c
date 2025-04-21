/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Grammar implementation */

#include <python/token.h>
#include <python/grammar.h>
#include <python/errors.h>

#include <asys/memory.h>
#include <asys/string.h>

struct py_grammar* py_grammar_new(int start) {
	struct py_grammar* grammar;

	grammar = asys_memory_allocate_zero(1, sizeof(struct py_grammar));
	/* TODO: Better EH. */
	if(!grammar) py_fatal("oom");

	grammar->start = start;

	return grammar;
}

struct py_dfa* py_grammar_add_dfa(
		struct py_grammar* grammar, int type, char* name) {

	struct py_dfa* dfa;

	grammar->dfas = asys_memory_reallocate_safe(
			grammar->dfas, (grammar->count + 1) * sizeof(struct py_dfa));

	if(!grammar->dfas) {
		/* TODO: Better EH. */
		py_fatal("oom");
	}

	dfa = &grammar->dfas[grammar->count++];
	dfa->type = type;
	dfa->name = name;
	dfa->count = 0;
	dfa->states = 0;
	dfa->initial = -1;
	dfa->first = 0;

	return dfa; /* Only use while fresh! */
}

unsigned py_dfa_add_state(struct py_dfa* dfa) {
	struct py_state* state;

	dfa->states = asys_memory_reallocate_safe(
			dfa->states, (dfa->count + 1) * sizeof(struct py_state));

	if(!dfa->states) {
		/* TODO: Better EH. */
		py_fatal("oom");
	}

	state = &dfa->states[dfa->count++];
	state->count = 0;
	state->arcs = 0;
	state->accel = 0;

	return (unsigned) (state - dfa->states);
}

void py_dfa_add_arc(
		struct py_dfa* dfa, unsigned from, unsigned to, unsigned lbl) {

	struct py_state* state;
	struct py_arc* a;

	/* TODO: Better EH. */
	if(from >= dfa->count) py_fatal("add_arc: error");
	if(to >= dfa->count) py_fatal("add_arc: error");

	state = &dfa->states[from];

	state->arcs = asys_memory_reallocate_safe(
			state->arcs, (state->count + 1) * sizeof(struct py_arc));

	if(!state->arcs) {
		/* TODO: Better EH. */
		py_fatal("oom");
	}

	a = &state->arcs[state->count++];
	a->label = (unsigned short) lbl;
	a->arrow = (unsigned short) to;
}

unsigned py_labellist_add(
		struct py_labellist* label_list, unsigned type, char* str) {

	unsigned i;
	struct py_label* label;

	for(i = 0; i < label_list->count; i++) {
		if(label_list->label[i].type == type &&
				asys_string_equal(label_list->label[i].str, str)) {

			return i;
		}
	}

	label_list->label = asys_memory_reallocate_safe(
			label_list->label,
			(label_list->count + 1) * sizeof(struct py_label));

	if(!label_list->label) {
		/* TODO: Better EH. */
		py_fatal("oom");
	}

	label = &label_list->label[label_list->count++];
	label->type = type;
	label->str = str; /* TODO: strdup(str) ??? */

	return (unsigned) (label - label_list->label);
}

/* Same, but rather dies than adds */

unsigned py_labellist_find(struct py_labellist* label_list, unsigned type, char* str) {
	unsigned i;

	for(i = 0; i < label_list->count; i++) {
		if(label_list->label[i].type == type /*&&
                       strcmp(label_list->label[i].str, str) == 0*/) {

			return i;
		}
	}

	/* TODO: Better EH. */
	py_fatal("Label dfa not found");
	(void) str;

	return ASYS_UINT_MAX;
}

static void py_grammar_translate_label(
		struct py_grammar* grammar, struct py_label* label) {

	unsigned i;

	if(label->type == PY_NAME) {
		for(i = 0; i < grammar->count; i++) {
			if(asys_string_equal(label->str, grammar->dfas[i].name)) {
				label->type = grammar->dfas[i].type;
				label->str = 0;

				return;
			}
		}

		for(i = 0; i < (int) PY_N_TOKENS; i++) {
			if(asys_string_equal(label->str, py_token_names[i])) {
				label->type = i;
				label->str = 0;

				return;
			}
		}

		/* TODO: Better EH. */
		py_fatal("Can't translate PY_NAME label");
	}

	if(label->type == PY_STRING) {
		if(asys_character_is_letter(label->str[1])) {
			char* p;

			label->type = PY_NAME;
			label->str++;

			p = asys_string_find(label->str, '\'');
			if(p) *p = '\0';
		}
		else {
			if(label->str[2] == label->str[0]) {
				int type = (int) py_token_char(label->str[1]);

				if(type != PY_OP) {
					label->type = type;
					label->str = 0;
				}
				/* TODO: Better EH. */
				else py_fatal("Unknown PY_OP label");
			}
			/* TODO: Better EH. */
			else py_fatal("Can't translate PY_STRING label");
		}
	}
	/* TODO: Better EH. */
	else py_fatal("Can't translate label");
}

void py_grammar_translate(struct py_grammar* grammar) {
	unsigned i;

	/* Don't translate EMPTY */
	for(i = PY_LABEL_EMPTY + 1; i < grammar->labels.count; i++) {
		py_grammar_translate_label(grammar, &grammar->labels.label[i]);
	}
}
