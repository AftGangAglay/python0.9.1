/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

#include <python/state.h>
#include <python/types.h>
#include <python/object/dict.h>

#include <asys/string.h>
#include <asys/memory.h>

static enum py_result py_path_new(const char* path, char*** out) {
	enum py_result res = PY_RESULT_OK;

	const char* curr = path;
	unsigned n = 1;

	*out = asys_memory_allocate_zero(1, sizeof(char*));
	if(!*out) return PY_RESULT_OOM;

	while(1) {
		unsigned len;
		char** s;

		const char* pos = asys_string_find_const(curr, ':');
		if(!pos) pos = asys_string_find_const(curr, '\0');

		len = (unsigned) (pos - path);
		s = &(*out)[n - 1];

		if(!(*s = asys_memory_allocate(len + 1))) {
			res = PY_RESULT_OOM;
			goto cleanup;
		}

		asys_memory_copy(*s, curr, len);
		(*s)[len] = 0;

		*out = asys_memory_reallocate_safe(*out, ++n * sizeof(char*));
		if(!*out) {
			res = PY_RESULT_OOM;
			goto cleanup;
		}

		(*out)[n - 1] = 0;

		if(!*pos) break;
		curr = pos + 1;
	}

	return PY_RESULT_OK;

	cleanup: {
		unsigned i;

		for(i = 0; i < n; ++i) {
			if(*out[i]) asys_memory_free(*out[i]);
		}

		asys_memory_free(*out);

		return res;
	}
}

enum py_result py_new(struct py* py, const char* path) {
	enum py_result res;

	if((res = py_types_register(py)) != PY_RESULT_OK) return res;
	if((res = py_path_new(path, &py->path)) != PY_RESULT_OK) return res;

	return PY_RESULT_OK;
}

enum py_result py_delete(struct py* py) {
	(void) py;
	return PY_RESULT_OK;
}

enum py_result py_env_new(struct py* py, struct py_env* env) {
	env->py = py;

	if(!(env->modules = py_dict_new())) return PY_RESULT_OOM;

	return PY_RESULT_OK;
}

enum py_result py_env_delete(struct py_env* env) {
	(void) env;
	return PY_RESULT_OK;
}
