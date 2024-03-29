/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

#include <python/compile.h>
#include <python/traceback.h>

#include <python/object.h>
#include <python/classobject.h>
#include <python/frameobject.h>
#include <python/funcobject.h>
#include <python/methodobject.h>
#include <python/moduleobject.h>
#include <python/tupleobject.h>
#include <python/listobject.h>
#include <python/stringobject.h>
#include <python/dictobject.h>
#include <python/intobject.h>
#include <python/floatobject.h>

static struct py_sequencemethods py_tuple_sequence = {
		py_tuple_cat, py_tuple_ind, py_tuple_slice,
};

struct py_sequencemethods py_list_sequence = {
		py_list_cat, py_list_ind, py_list_slice
};

struct py_sequencemethods py_string_sequence = {
		py_string_cat, py_string_ind, py_string_slice
};

struct py_type_info py_types[PY_TYPE_MAX] = {
		/* Type */
		{ sizeof(struct py_type_info), 0, 0, 0 },
		/* None */
		{ 0 },

		/* Class */
		{ sizeof(struct py_class), py_class_dealloc, 0, 0 },
		/* Class Member */
		{ sizeof(struct py_class_member), py_class_member_dealloc, 0, 0 },
		/* Class Method */
		{ sizeof(struct py_class_method), py_class_method_dealloc, 0, 0 },

		/* Code */
		{ sizeof(struct py_code), py_code_dealloc, 0, 0 },
		/* Frame */
		{ sizeof(struct py_frame), py_frame_dealloc, 0, 0 },
		/* Traceback */
		{ sizeof(struct py_traceback), py_traceback_dealloc, 0, 0 },
		/* Func */
		{ sizeof(struct py_func), py_func_dealloc, 0, 0 },
		/* Method */
		{ sizeof(struct py_method), py_method_dealloc, 0, 0 },
		/* Module */
		{ sizeof(struct py_module), py_module_dealloc, 0, 0 },

		/* Tuple */
		{
				sizeof(struct py_tuple),
				py_tuple_dealloc, py_tuple_cmp, &py_tuple_sequence
		},
		/* List */
		{
				sizeof(struct py_list),
				py_list_dealloc, py_list_cmp, &py_list_sequence
		},
		/* String */
		{
				sizeof(struct py_string),
				py_object_delete, py_string_cmp, &py_string_sequence
		},

		/* Dict */
		{
				sizeof(struct py_dict),
				py_dict_dealloc, 0, 0
		},

		/* Int */
		{
				sizeof(struct py_int),
				py_int_dealloc, py_int_cmp, 0
		},
		/* Float */
		{
				sizeof(struct py_float),
				py_object_delete, py_float_cmp, 0
		},
};
