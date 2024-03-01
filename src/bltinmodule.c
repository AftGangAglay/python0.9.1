/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Built-in functions */

#include <python/node.h>
#include <python/graminit.h>
#include <python/import.h>
#include <python/modsupport.h>
#include <python/result.h>
#include <python/fgetsintr.h>
#include <python/ceval.h>
#include <python/errors.h>

#include <python/sysmodule.h>
#include <python/bltinmodule.h>

#include <python/frameobject.h>
#include <python/intobject.h>
#include <python/floatobject.h>
#include <python/stringobject.h>
#include <python/moduleobject.h>
#include <python/listobject.h>
#include <python/dictobject.h>
#include <python/tupleobject.h>
#include <python/fileobject.h>

/* Predefined exceptions */

struct py_object* py_runtime_error;
struct py_object* py_eof_error;
struct py_object* py_type_error;
struct py_object* py_memory_error;
struct py_object* py_name_error;
struct py_object* py_system_error;
struct py_object* py_interrupt_error;

static struct py_object* builtin_abs(struct py_object* self, struct py_object* v) {
	(void) self;

	/* XXX This should be a method in the as_number struct in the type */
	if(v == NULL) {
		/* */
	}
	else if(py_is_int(v)) {
		long x = py_int_get(v);

		if(x < 0) x = -x;
		return py_int_new(x);
	}
	else if(py_is_float(v)) {
		double x = py_float_get(v);

		if(x < 0) x = -x;
		return py_float_new(x);
	}

	py_error_set_string(py_type_error, "abs() argument must be float or int");
	return NULL;
}

static struct py_object* builtin_chr(struct py_object* self, struct py_object* v) {
	long x;
	char s[1];

	(void) self;

	if(v == NULL || !py_is_int(v)) {
		py_error_set_string(py_type_error, "chr() must have int argument");
		return NULL;
	}

	x = py_int_get(v);

	if(x < 0 || x >= 256) {
		py_error_set_string(py_runtime_error, "chr() arg not in range(256)");
		return NULL;
	}

	s[0] = x;
	return py_string_new_size(s, 1);
}

static struct py_object* builtin_dir(struct py_object* self, struct py_object* v) {
	struct py_object* d;

	(void) self;

	if(v == NULL) d = py_get_locals();
	else {
		if(!py_is_module(v)) {
			py_error_set_string(py_type_error, "dir() argument, must be module or absent");
			return NULL;
		}

		d = py_module_get_dict(v);
	}

	v = py_dict_get_keys(d);
	if(py_list_sort(v) != 0) {
		PY_DECREF(v);
		v = NULL;
	}

	return v;
}

static struct py_object* builtin_divmod(struct py_object* self, struct py_object* v) {
	struct py_object* x;
	struct py_object* y;
	long xi, yi, xdivy, xmody;

	(void) self;

	if(v == NULL || !py_is_tuple(v) || py_tuple_size(v) != 2) {
		py_error_set_string(py_type_error, "divmod() requires 2 int arguments");
		return NULL;
	}

	x = py_tuple_get(v, 0);
	y = py_tuple_get(v, 1);

	if(!py_is_int(x) || !py_is_int(y)) {
		py_error_set_string(py_type_error, "divmod() requires 2 int arguments");
		return NULL;
	}

	xi = py_int_get(x);
	yi = py_int_get(y);

	if(yi == 0) {
		py_error_set_string(py_type_error, "divmod() division by zero");
		return NULL;
	}

	if(yi < 0) { xdivy = -xi / -yi; }
	else { xdivy = xi / yi; }

	xmody = xi - xdivy * yi;

	if((xmody < 0 && yi > 0) || (xmody > 0 && yi < 0)) {
		xmody += yi;
		xdivy -= 1;
	}

	v = py_tuple_new(2);
	x = py_int_new(xdivy);
	y = py_int_new(xmody);

	if(v == NULL || x == NULL || y == NULL || py_tuple_set(v, 0, x) != 0 ||
	   py_tuple_set(v, 1, y) != 0) {

		PY_XDECREF(v);
		PY_XDECREF(x);
		PY_XDECREF(y);

		return NULL;
	}

	return v;
}

static struct py_object* builtin_float(struct py_object* self, struct py_object* v) {
	(void) self;

	if(v == NULL) {
		/* */
	}
	else if(py_is_float(v)) {
		PY_INCREF(v);
		return v;
	}
	else if(py_is_int(v)) {
		long x = py_int_get(v);
		return py_float_new((double) x);
	}

	py_error_set_string(py_type_error, "float() argument must be float or int");
	return NULL;
}

static struct py_object* builtin_int(struct py_object* self, struct py_object* v) {
	(void) self;

	if(v == NULL) {
		/* */
	}
	else if(py_is_int(v)) {
		PY_INCREF(v);
		return v;
	}
	else if(py_is_float(v)) {
		double x = py_float_get(v);
		return py_int_new((long) x);
	}

	py_error_set_string(py_type_error, "int() argument must be float or int");
	return NULL;
}

static struct py_object* builtin_len(struct py_object* self, struct py_object* v) {
	long len;
	struct py_type * tp;

	(void) self;

	if(v == NULL) {
		py_error_set_string(py_type_error, "len() without argument");
		return NULL;
	}

	tp = v->type;

	if(tp->sequencemethods != NULL) {
		len = (*tp->sequencemethods->len)(
				v);
	}
	else if(tp->mappingmethods != NULL) {
		len = (*tp->mappingmethods->len)(v);
	}
	else {
		py_error_set_string(py_type_error, "len() of unsized object");
		return NULL;
	}

	return py_int_new(len);
}

static struct py_object* min_max(struct py_object* v, int sign) {
	int i, n, cmp;
	struct py_object* w;
	struct py_object* x;
	struct py_sequencemethods* sq;

	if(v == NULL) {
		py_error_set_string(py_type_error, "min() or max() without argument");
		return NULL;
	}

	sq = v->type->sequencemethods;

	if(sq == NULL) {
		py_error_set_string(py_type_error, "min() or max() of non-sequence");
		return NULL;
	}

	n = (*sq->len)(v);

	if(n == 0) {
		py_error_set_string(py_runtime_error, "min() or max() of empty sequence");
		return NULL;
	}

	w = (*sq->ind)(v, 0); /* Implies PY_INCREF */

	for(i = 1; i < n; i++) {
		x = (*sq->ind)(v, i); /* Implies PY_INCREF */
		cmp = py_object_cmp(x, w);

		if(cmp * sign > 0) {
			PY_DECREF(w);
			w = x;
		}
		else PY_DECREF(x);
	}

	return w;
}

static struct py_object* builtin_min(struct py_object* self, struct py_object* v) {
	(void) self;

	return min_max(v, -1);
}

static struct py_object* builtin_max(struct py_object* self, struct py_object* v) {
	(void) self;

	return min_max(v, 1);
}

static struct py_object* builtin_open(struct py_object* self, struct py_object* v) {
	struct py_object* name;
	struct py_object* mode;

	(void) self;

	if(v == NULL || !py_is_tuple(v) || py_tuple_size(v) != 2 ||
	   !py_is_string(name = py_tuple_get(v, 0)) ||
	   !py_is_string(mode = py_tuple_get(v, 1))) {

		py_error_set_string(py_type_error, "open() requires 2 string arguments");
		return NULL;
	}

	v = py_file_new(py_string_get_value(name), py_string_get_value(mode));
	return v;
}

static struct py_object* builtin_ord(struct py_object* self, struct py_object* v) {
	(void) self;

	if(v == NULL || !py_is_string(v)) {
		py_error_set_string(py_type_error, "ord() must have string argument");
		return NULL;
	}

	if(py_string_size(v) != 1) {
		py_error_set_string(py_runtime_error, "ord() arg must have length 1");
		return NULL;
	}

	return py_int_new((long) (py_string_get_value(v)[0] & 0xff));
}

static struct py_object* builtin_range(struct py_object* self, struct py_object* v) {
	static char* errmsg = "range() requires 1-3 int arguments";
	int i, n;
	long ilow, ihigh, istep;

	(void) self;

	if(v != NULL && py_is_int(v)) {
		ilow = 0;
		ihigh = py_int_get(v);
		istep = 1;
	}
	else if(v == NULL || !py_is_tuple(v)) {
		py_error_set_string(py_type_error, errmsg);
		return NULL;
	}
	else {
		n = py_tuple_size(v);

		if(n < 1 || n > 3) {
			py_error_set_string(py_type_error, errmsg);
			return NULL;
		}

		for(i = 0; i < n; i++) {
			if(!py_is_int(py_tuple_get(v, i))) {
				py_error_set_string(py_type_error, errmsg);
				return NULL;
			}
		}

		if(n == 3) {
			istep = py_int_get(py_tuple_get(v, 2));
			--n;
		}
		else { istep = 1; }

		ihigh = py_int_get(py_tuple_get(v, --n));

		if(n > 0) { ilow = py_int_get(py_tuple_get(v, 0)); }
		else { ilow = 0; }
	}

	if(istep == 0) {
		py_error_set_string(py_runtime_error, "zero step for range()");
		return NULL;
	}

	/* XXX ought to check overflow of subion */
	if(istep > 0) { n = (ihigh - ilow + istep - 1) / istep; }
	else { n = (ihigh - ilow + istep + 1) / istep; }

	if(n < 0) n = 0;

	v = py_list_new(n);
	if(v == NULL) return NULL;

	for(i = 0; i < n; i++) {
		struct py_object* w = py_int_new(ilow);

		if(w == NULL) {
			PY_DECREF(v);
			return NULL;
		}

		py_list_set(v, i, w);
		ilow += istep;
	}

	return v;
}

static struct py_object* builtin_raw_input(struct py_object* self, struct py_object* v) {
	FILE* in = py_system_get_file("stdin", stdin);
	FILE* out = py_system_get_file("stdout", stdout);
	int err;
	int n = 1000;

	(void) self;

	if(v != NULL) py_object_print(v, out, PY_PRINT_RAW);

	v = py_string_new_size((char*) NULL, n);

	if(v != NULL) {
		if((err = py_fgets_intr(py_string_get_value(v), n + 1, in)) != PY_RESULT_OK) {
			py_error_set_input(err);
			PY_DECREF(v);
			return NULL;
		}
		else {
			n = strlen(py_string_get_value(v));

			if(n > 0 && py_string_get_value(v)[n - 1] == '\n') n--;

			py_string_resize(&v, n);
		}
	}

	return v;
}

static struct py_object* builtin_reload(struct py_object* self, struct py_object* v) {
	(void) self;

	return py_reload_module(v);
}

static struct py_object* builtin_type(struct py_object* self, struct py_object* v) {
	(void) self;

	if(v == NULL) {
		py_error_set_string(py_type_error, "type() requres an argument");
		return NULL;
	}

	v = (struct py_object*) v->type;
	PY_INCREF(v);
	return v;
}

static struct py_methodlist builtin_methods[] = {
		{ "abs",       builtin_abs },
		{ "chr",       builtin_chr },
		{ "dir",       builtin_dir },
		{ "divmod",    builtin_divmod },
		{ "float",     builtin_float },
		{ "int",       builtin_int },
		{ "len",       builtin_len },
		{ "max",       builtin_max },
		{ "min",       builtin_min },
		{ "open",      builtin_open }, /* XXX move to OS module */
		{ "ord",       builtin_ord },
		{ "range",     builtin_range },
		{ "raw_input", builtin_raw_input },
		{ "reload",    builtin_reload },
		{ "type",      builtin_type },
		{ NULL, NULL }, };

/* TODO: Centralise all global interpreter state. */
static struct py_object* builtin_dict;

struct py_object* py_builtin_get(const char* name) {
	return py_dict_lookup(builtin_dict, name);
}

static struct py_object* newstdexception(char* name, char* message) {
	struct py_object* v = py_string_new(message);

	if(v == NULL || py_dict_insert(builtin_dict, name, v) != 0) {
		py_fatal("no mem for new standard exception");
	}

	return v;
}

static void initerrors(void) {
	py_runtime_error = newstdexception("py_runtime_error", "run-time error");
	py_eof_error = newstdexception("py_eof_error", "end-of-file read");
	py_type_error = newstdexception("py_type_error", "type error");
	py_memory_error = newstdexception("py_memory_error", "out of memory");
	py_name_error = newstdexception("py_name_error", "undefined name");
	py_system_error = newstdexception("py_system_error", "system error");
	py_interrupt_error = newstdexception(
			"py_interrupt_error", "keyboard interrupt");
}

void py_errors_done(void) {
	PY_DECREF(py_runtime_error);
	PY_DECREF(py_eof_error);
	PY_DECREF(py_type_error);
	PY_DECREF(py_memory_error);
	PY_DECREF(py_name_error);
	PY_DECREF(py_system_error);
	PY_DECREF(py_interrupt_error);
}

void py_builtin_init(void) {
	struct py_object* m;

	m = py_module_init("builtin", builtin_methods);
	builtin_dict = py_module_get_dict(m);
	PY_INCREF(builtin_dict);

	initerrors();
	(void) py_dict_insert(builtin_dict, "PY_NONE", PY_NONE);
}

void py_builtin_done(void) {
	PY_DECREF(builtin_dict);
}
