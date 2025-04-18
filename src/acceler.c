/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Parser accelerator module */

/* The parser as originally conceived had disappointing performance.
   This module does some precomputation that speeds up the selection
   of a DFA based upon a token, turning a search through an array
   into a simple indexing operation. The parser now cannot work
   without the accelerators installed. Note that the accelerators
   are installed dynamically when the parser is initialized, they
   are not part of the static data structure written on graminit.[ch]
   by the parser generator. */

#include <python/grammar.h>
#include <python/token.h>
#include <python/parser.h>

#include <asys/memory.h>
#include <asys/log.h>

/* TODO: This isn't a great system. */
static int** py_grammar_freelist = 0;
static unsigned py_grammar_freelist_len = 0;

static int py_grammar_fix_state(struct py_grammar* g, struct py_state* s) {
	struct py_arc* a;
	unsigned k;
	int* accel;
	unsigned nl = g->labels.count;

	s->accept = 0;

	accel = asys_memory_allocate(nl * sizeof(int));
	if(!accel) return -1;

	for(k = 0; k < nl; k++) accel[k] = -1;
	a = s->arcs;

	for(k = 0; k < s->count; ++k, ++a) {
		unsigned lbl = a->label;
		struct py_label* l = &g->labels.label[lbl];
		unsigned type = l->type;

		if(a->arrow >= (1 << 7)) {
			asys_log(__FILE__, "warn: Too many states");
			continue;
		}

		if(type >= PY_NONTERMINAL) {
			struct py_dfa* d1 = py_grammar_find_dfa(g, type);
			unsigned bit;

			if(type - PY_NONTERMINAL >= (1 << 7)) {
				asys_log(__FILE__, "warn: Non-terminal number too high");
				continue;
			}

			for(bit = 0; bit < g->labels.count; bit++) {
				if(PY_TESTBIT(d1->first, bit)) {
					if(accel[bit] != -1) {
						asys_log(__FILE__, "warn: Accelerator ambiguity");
					}

					accel[bit] = a->arrow | (1 << 7);
					accel[bit] |= ((type - PY_NONTERMINAL) << 8);
				}
			}
		}
		else if(lbl == PY_LABEL_EMPTY) { s->accept = 1; }
		else if(lbl < nl) accel[lbl] = a->arrow;
	}

	while(nl > 0 && accel[nl - 1] == -1) nl--;
	for(k = 0; k < nl && accel[k] == -1;) k++;

	if(k < nl) {
		int i;

		s->accel = asys_memory_allocate((nl - k) * sizeof(int));
		if(!s->accel) return -1;

		py_grammar_freelist = asys_memory_reallocate_safe(
				py_grammar_freelist, ++py_grammar_freelist_len * sizeof(int*));

		if(!py_grammar_freelist) return -1;

		py_grammar_freelist[py_grammar_freelist_len - 1] = s->accel;
		s->lower = k;
		s->upper = nl;

		for(i = 0; k < nl; i++, k++) s->accel[i] = accel[k];
	}

	asys_memory_free(accel);

	return 0;
}

static void py_grammar_fix_dfa(struct py_grammar* g, struct py_dfa* d) {
	struct py_state* s;
	unsigned j;

	s = d->states;
	for(j = 0; j < d->count; j++, s++) py_grammar_fix_state(g, s);
}

void py_grammar_add_accels(struct py_grammar* g) {
	struct py_dfa* d;
	unsigned i;

	d = g->dfas;

	for(i = 0; i < g->count; ++i, ++d) py_grammar_fix_dfa(g, d);

	g->accel = 1;
}

void py_grammar_delete_accels(void) {
	unsigned i;

	for(i = 0; i < py_grammar_freelist_len; ++i) {
		asys_memory_free(py_grammar_freelist[i]);
	}

	asys_memory_free(py_grammar_freelist);
}
