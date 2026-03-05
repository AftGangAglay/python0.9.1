/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Float object implementation */

#include <python/object/float.h>

#include <asys/memory.h>

struct py_object* py_float_new(double value) {
	struct py_float* op;

	if(!(op = py_object_new(PY_TYPE_FLOAT))) return 0;

	op->value = value;

	return (void*) op;
}

void py_float_dealloc(struct py_object* op) {
	asys_memory_free(op);
}

double py_float_get(const struct py_object* op) {
	return ((struct py_float*) op)->value;
}

/* Methods */

int py_float_cmp(const struct py_object* v, const struct py_object* w) {
	double i = py_float_get(v);
	double j = py_float_get(w);

	return (i < j) ? -1 : (i > j) ? 1 : 0;
}

enum asys_result py_float_serialize(
		struct py_object* object, struct py_serialization_context* context) {

	enum asys_result result;
	struct py_float* float_object = (struct py_float*) object;

	result = asys_stream_write(
			context->stream, 0, &float_object->value, sizeof(double));

	if(result) return result;

	return ASYS_RESULT_OK;
}
