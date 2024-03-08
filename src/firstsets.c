/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Computation of FIRST stets */

#include <python/errors.h>
#include <python/grammar.h>
#include <python/token.h>

extern int debugging;

/* Forward */
static void py_grammar_calculate_first_set(struct py_grammar*, struct py_dfa*);

void py_grammar_add_firsts(struct py_grammar* g) {
	unsigned i;
	struct py_dfa* d;

	fprintf(stderr, "Adding FIRST sets ...\n");
	for(i = 0; i < g->count; i++) {
		d = &g->dfas[i];
		if(d->first == NULL) {
			py_grammar_calculate_first_set(g, d);
		}
	}
}

/* TODO: Give this a cleanup. */
static void py_grammar_calculate_first_set(
		struct py_grammar* g, struct py_dfa* d) {

	unsigned i, j;
	struct py_state* s;
	struct py_arc* a;
	unsigned nbits;
	unsigned nsyms;
	int* sym;
	static py_bitset_t dummy;
	py_bitset_t result;
	int type;
	struct py_dfa* d1;
	struct py_label* l0;

	if(debugging) printf("Calculate FIRST set for '%s'\n", d->name);

	if(dummy == NULL) dummy = py_bitset_new(1);

	if(d->first == dummy) {
		fprintf(stderr, "Left-recursion for '%s'\n", d->name);
		return;
	}

	if(d->first != NULL) {
		fprintf(stderr, "Re-calculating FIRST set for '%s' ???\n", d->name);
	}

	d->first = dummy;

	l0 = g->labels.label;
	nbits = g->labels.count;
	result = py_bitset_new(nbits);

	sym = malloc(sizeof(int));
	if(sym == NULL) {
		py_fatal("no mem for new sym in py_grammar_calculate_first_set");
	}

	nsyms = 1;
	sym[0] = py_labellist_find(&g->labels, d->type, NULL);

	s = &d->states[d->initial];

	for(i = 0; i < s->count; i++) {
		a = &s->arcs[i];

		for(j = 0; j < nsyms; j++) {
			if(sym[j] == a->label) break;
		}

		if(j >= nsyms) { /* New label */
			/* TODO: Leaky realloc. */
			sym = realloc(sym, (nsyms + 1) * sizeof(int));
			if(sym == NULL) {
				py_fatal(
						"no mem to resize sym in "
						"py_grammar_calculate_first_set");
			}

			sym[nsyms++] = a->label;
			type = l0[a->label].type;

			if(type >= PY_NONTERMINAL) {
				d1 = py_grammar_find_dfa(g, type);

				if(d1->first == dummy) {
					fprintf(stderr, "Left-recursion below '%s'\n", d->name);
				}
				else {
					if(d1->first == NULL) {
						py_grammar_calculate_first_set(g, d1);
					}
					py_bitset_merge(result, d1->first, nbits);
				}
			}
			else py_bitset_add(result, a->label);
		}
	}
	d->first = result;
	if(debugging) {
		printf("FIRST set for '%s': {", d->name);
		for(i = 0; i < nbits; i++) {
			if(PY_TESTBIT(result, i)) {
				printf(" %s", py_label_repr(&l0[i]));
			}
		}
		printf(" }\n");
	}
}
