/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Module support implementation */

#include <python/modsupport.h>
#include <python/errors.h>
#include <python/import.h>

#include <python/moduleobject.h>
#include <python/dictobject.h>
#include <python/tupleobject.h>
#include <python/stringobject.h>
#include <python/intobject.h>
#include <python/floatobject.h>

/* TODO: Better EH. */
struct py_object* py_module_new_methods(
		const char* name, const struct py_methodlist* methods) {

	struct py_object* m;
	struct py_object* d;
	struct py_object* v;
	const struct py_methodlist* ml;

	if((m = py_module_add(name)) == NULL) {
		fprintf(stderr, "initializing module: %s\n", name);
		py_fatal("can't create a module");
	}

	d = ((struct py_module*) m)->attr;

	for(ml = methods; ml->name != NULL; ml++) {
		v = py_method_new(ml->method, (struct py_object*) NULL);

		if(v == NULL || py_dict_insert(d, ml->name, v) != 0) {
			fprintf(stderr, "initializing module: %s\n", name);
			py_fatal("can't initialize module");
		}

		py_object_decref(v);
	}

	return m;
}

/* Argument list handling tools.
   All return 1 for success, or call py_error_set*() and return 0 for failure */

int py_arg_none(struct py_object* v) {
	if(v != NULL) return py_error_set_badarg();

	return 1;
}

int py_arg_int(struct py_object* v, int* a) {
	if(v == NULL || !(v->type == PY_TYPE_INT)) {
		return py_error_set_badarg();
	}
	*a = (int) py_int_get(v);
	return 1;
}

int py_arg_int_int(struct py_object* v, int* a, int* b) {
	if(v == NULL || !(v->type == PY_TYPE_TUPLE) || py_varobject_size(v) != 2) {
		return py_error_set_badarg();
	}
	return py_arg_int(py_tuple_get(v, 0), a) &&
		   py_arg_int(py_tuple_get(v, 1), b);
}

int py_arg_long(struct py_object* v, long* a) {
	if(v == NULL || !(v->type == PY_TYPE_INT)) {
		return py_error_set_badarg();
	}
	*a = (long) py_int_get(v);
	return 1;
}

int py_arg_long_long(struct py_object* v, long* a, long* b) {
	if(v == NULL || !(v->type == PY_TYPE_TUPLE) || py_varobject_size(v) != 2) {
		return py_error_set_badarg();
	}
	return py_arg_long(py_tuple_get(v, 0), a) &&
		   py_arg_long(py_tuple_get(v, 1), b);
}

int py_arg_str(struct py_object* v, struct py_object** a) {
	if(v == NULL || !(v->type == PY_TYPE_STRING)) {
		return py_error_set_badarg();
	}
	*a = v;
	return 1;
}

int py_arg_str_str(
		struct py_object* v, struct py_object** a, struct py_object** b) {
	if(v == NULL || !(v->type == PY_TYPE_TUPLE) || py_varobject_size(v) != 2) {
		return py_error_set_badarg();
	}
	return py_arg_str(py_tuple_get(v, 0), a) &&
		   py_arg_str(py_tuple_get(v, 1), b);
}

int py_arg_str_int(struct py_object* v, struct py_object** a, int* b) {
	if(v == NULL || !(v->type == PY_TYPE_TUPLE) || py_varobject_size(v) != 2) {
		return py_error_set_badarg();
	}
	return py_arg_str(py_tuple_get(v, 0), a) &&
		   py_arg_int(py_tuple_get(v, 1), b);
}

int getpointarg(struct py_object* v, int* a /* [2] */) {
	return py_arg_int_int(v, a, a + 1);
}
