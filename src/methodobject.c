/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Method object implementation */

#include <python/token.h>
#include <python/errors.h>

#include <python/object.h>
#include <python/stringobject.h>
#include <python/methodobject.h>

typedef struct {
	struct py_object ob;
	char* m_name;
	unsigned m_heap_name;
	py_method_t m_meth;
	struct py_object* m_self;
} methodobject;

struct py_object* py_method_new(name, meth, self, heapname)
		char* name; /* static string */
		py_method_t meth;
		struct py_object* self;
		unsigned heapname;
{
	methodobject* op = py_object_new(&py_method_type);
	if(op != NULL) {
		op->m_heap_name = heapname;
		op->m_name = name;
		op->m_meth = meth;
		if(self != NULL)
			py_object_incref(self);
		op->m_self = self;
	}
	return (struct py_object*) op;
}

py_method_t py_method_get(op)struct py_object* op;
{
	if(!py_is_method(op)) {
		py_error_set_badcall();
		return NULL;
	}
	return ((methodobject*) op)->m_meth;
}

struct py_object* py_method_get_self(op)struct py_object* op;
{
	if(!py_is_method(op)) {
		py_error_set_badcall();
		return NULL;
	}
	return ((methodobject*) op)->m_self;
}

/* Methods (the standard built-in methods, that is) */

static void meth_dealloc(m)methodobject* m;
{
	if(m->m_heap_name) free(m->m_name);
	if(m->m_self != NULL) {
		py_object_decref(m->m_self);
	}
	free(m);
}

static void meth_print(m, fp, flags)methodobject* m;
									FILE* fp;
									int flags;
{
	(void) flags;

	if(m->m_self == NULL) {
		fprintf(fp, "<built-in function '%s'>", m->m_name);
	}
	else {
		fprintf(
				fp, "<built-in method '%s' of some %s object>", m->m_name,
				m->m_self->type->name);
	}
}

struct py_type py_method_type = {
		{ 1, &py_type_type, 0 }, "method", sizeof(methodobject),
		meth_dealloc, /* dealloc */
		meth_print, /* print */
		0, /* get_attr */
		0, /* set_attr */
		0, /* cmp */
		0, /* numbermethods */
		0, /* sequencemethods */
		0, /* mappingmethods */
};

/* Find a method in a module's method table.
   Usually called from an object's py_object_get_attr method. */

struct py_object* py_methodlist_find(ml, op, name)struct py_methodlist* ml;
												  struct py_object* op;
												  const char* name;
{
	for(; ml->name != NULL; ml++) {
		if(strcmp(name, ml->name) == 0) {
			return py_method_new(ml->name, ml->method, op, 0);
		}
	}
	py_error_set_string(py_name_error, name);
	return NULL;
}
