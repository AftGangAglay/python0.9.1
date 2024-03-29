/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* String object implementation */

#include <python/std.h>
#include <python/stringobject.h>
#include <python/errors.h>

struct py_object* py_string_new_size(const char* str, unsigned size) {

	struct py_string* op = malloc(
			sizeof(struct py_string) + size * sizeof(char));
	if(op == NULL) return py_error_set_nomem();

	py_object_newref(op);
	op->ob.type = PY_TYPE_STRING;
	op->ob.size = size;

	if(str != NULL) memcpy(op->value, str, size);

	op->value[size] = '\0';

	return (struct py_object*) op;
}

struct py_object* py_string_new(const char* str) {
	unsigned size = strlen(str);
	struct py_string* op = malloc(
			sizeof(struct py_string) + size * sizeof(char));

	if(op == NULL) return py_error_set_nomem();

	py_object_newref(op);

	op->ob.type = PY_TYPE_STRING;
	op->ob.size = size;

	strcpy(op->value, str); /* TODO: What is this for? */

	return (struct py_object*) op;
}

const char* py_string_get(const struct py_object* op) {
	if(!(op->type == PY_TYPE_STRING)) {
		py_error_set_badcall();
		return NULL;
	}

	return ((struct py_string*) op)->value;
}

/* Methods */

struct py_object* py_string_cat(struct py_object* a, struct py_object* b) {
	struct py_string* op;
	unsigned size;

	if(!(b->type == PY_TYPE_STRING)) {
		py_error_set_badarg();
		return NULL;
	}

	/* Optimize cases with empty left or right operand */
	if(py_varobject_size(a) == 0) {
		py_object_incref(b);
		return b;
	}

	if(py_varobject_size(b) == 0) {
		py_object_incref(a);
		return a;
	}

	size = py_varobject_size(a) + py_varobject_size(b);

	/* TODO: Not using _new_size? */
	op = malloc(sizeof(struct py_string) + size * sizeof(char));
	if(op == NULL) return py_error_set_nomem();

	py_object_newref(op);

	op->ob.type = PY_TYPE_STRING;
	op->ob.size = size;

	memcpy(op->value, py_string_get(a), py_varobject_size(a));
	memcpy(
			op->value + py_varobject_size(a), py_string_get(b),
			py_varobject_size(b));

	op->value[size] = '\0';

	return (struct py_object*) op;
}

/* String slice a[i:j] consists of characters a[i] ... a[j-1] */

struct py_object* py_string_slice(
		struct py_object* op, unsigned i, unsigned j) {

	if(j > py_varobject_size(op)) j = py_varobject_size(op);

	/* It's the same as op */
	if(i == 0 && j == py_varobject_size(op)) {
		py_object_incref(op);
		return (struct py_object*) op;
	}

	if(j < i) j = i;

	return py_string_new_size(py_string_get(op) + i, j - i);
}

struct py_object* py_string_ind(struct py_object* a, unsigned i) {
	/* TODO: Unchecked. */
	if(i >= py_varobject_size(a)) {
		py_error_set_string(PY_INDEX_ERROR, "string index out of range");
		return NULL;
	}

	return py_string_slice(a, i, i + 1);
}

int py_string_cmp(const struct py_object* a, const struct py_object* b) {

	unsigned len_a = py_varobject_size(a);
	unsigned len_b = py_varobject_size(b);
	unsigned min_len = (len_a < len_b) ? len_a : len_b;

	int cmp = memcmp(py_string_get(a), py_string_get(b), min_len);
	if(cmp != 0) return cmp;

	if(len_a < len_b) return -1;
	if(len_a > len_b) return 1;

	return 0;
}
