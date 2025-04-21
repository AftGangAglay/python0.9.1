/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Parse tree node interface */

#ifndef PY_NODE_H
#define PY_NODE_H

struct py_env;

struct py_node {
	int type;
	char* str;
	unsigned lineno;

	unsigned count;
	struct py_node* children;
};

struct py_node* py_tree_new(int);

void py_tree_delete(struct py_node* n);

struct py_node* py_tree_add(struct py_node*, int, char*, unsigned);

struct py_object* py_tree_run(
		struct py_env* env, struct py_node*, const char*, struct py_object*,
		struct py_object*);

struct py_object* py_tree_eval(
		struct py_env* env, struct py_node*, const char*, struct py_object*,
		struct py_object*);

/* Assert that the type of node is what we expect */
/* TODO: Handle this better. */
#ifndef NDEBUG
# define PY_REQ(n, t) \
		if((n)->type != (t)) py_fatal("FATAL: unexpected node type")

#else
# define PY_REQ(n, type) do { /* pass */ } while(0)
#endif

#endif
