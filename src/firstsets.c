/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Computation of FIRST stets */

#include <python/errors.h>
#include <python/grammar.h>
#include <python/token.h>

#include <asys/log.h>
#include <asys/memory.h>

static int py_grammar_calculate_first_set(
		struct py_grammar* g, struct py_dfa* d) {

	static py_bitset_t dummy;

	unsigned i, j, nbits, nsyms, type;

	unsigned* sym;

	py_bitset_t result;

	struct py_state* s;
	struct py_arc* a;
	struct py_dfa* d1;
	struct py_label* l0;

	/* TODO: This doesn't appear to get freed. */
	if(!(dummy = py_bitset_new(1))) py_fatal("out of memory");

	if(d->first == dummy) {
		asys_log(__FILE__, "warn: Left-recursion for '%s'", d->name);
		return 0;
	}

	if(d->first) {
		asys_log(
				__FILE__, "warn: Re-calculating FIRST set for '%s' ???",
				d->name);
	}

	d->first = dummy;

	l0 = g->labels.label;
	nbits = g->labels.count;

	result = py_bitset_new(nbits);
	if(!result) return -1;

	sym = asys_memory_allocate(sizeof(unsigned));
	if(!sym) {
		py_bitset_delete(result);
		return -1;
	}

	nsyms = 1;
	sym[0] = py_labellist_find(&g->labels, d->type, 0);

	s = &d->states[d->initial];

	for(i = 0; i < s->count; i++) {
		a = &s->arcs[i];

		for(j = 0; j < nsyms; j++) {
			if(sym[j] == a->label) break;
		}

		if(j >= nsyms) { /* New label */
			sym = asys_memory_reallocate_safe(
					sym, (nsyms + 1) * sizeof(unsigned));

			if(!sym) return -1;

			sym[nsyms++] = a->label;
			type = l0[a->label].type;

			if(type >= PY_NONTERMINAL) {
				d1 = py_grammar_find_dfa(g, type);

				if(d1->first == dummy) {
					asys_log(
							__FILE__,
							"warn: Left-recursion for '%s'", d->name);
				}
				else {
					if(!d1->first) {
						py_grammar_calculate_first_set(g, d1);
					}

					py_bitset_merge(result, d1->first, nbits);
				}
			}
			else py_bitset_add(result, a->label);
		}
	}

	d->first = result;

	return 0;
}

int py_grammar_add_firsts(struct py_grammar* g) {
	unsigned i;
	struct py_dfa* d;

	for(i = 0; i < g->count; i++) {
		d = &g->dfas[i];

		if(!d->first) {
			if(py_grammar_calculate_first_set(g, d) == -1) return -1;
		}
	}

	return 0;
}
