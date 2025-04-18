/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Parse tree node implementation */

#include <python/node.h>

#include <asys/memory.h>

struct py_node* py_tree_new(int type) {
	struct py_node* n = asys_memory_allocate_zero(1, sizeof(struct py_node));
	if(!n) return 0;

	n->type = type;

	return n;
}

/* Node alignment factor to speed up realloc */
#define PY_SCALE_FACTOR (3)
#define PY_ROUND_UP(n) \
	((n) == 1 ? 1 : \
		((n) + PY_SCALE_FACTOR - 1) / PY_SCALE_FACTOR * PY_SCALE_FACTOR)

struct py_node* py_tree_add(
		struct py_node* n1, int type, char* str, unsigned lineno) {

	unsigned nch = n1->count;
	unsigned nch1 = nch + 1;
	struct py_node* n;

	if(PY_ROUND_UP(nch) < nch1) {
		n = n1->children;
		nch1 = PY_ROUND_UP(nch1);

		n = asys_memory_reallocate_safe(n, nch1 * sizeof(struct py_node));
		if(!n) return 0;

		n1->children = n;
	}

	n = &n1->children[n1->count++];
	n->type = type;
	n->str = str;
	n->lineno = lineno;
	n->count = 0;
	n->children = 0;

	return n;
}

static void py_tree_free_children(struct py_node* n) {
	unsigned i;

	for(i = 0; i < n->count; ++i) py_tree_free_children(&n->children[i]);

	if(n->children) asys_memory_free(n->children);
	if(n->str) asys_memory_free(n->str);
}

void py_tree_delete(struct py_node* n) {
	if(n) {
		py_tree_free_children(n);
		asys_memory_free(n);
	}
}
