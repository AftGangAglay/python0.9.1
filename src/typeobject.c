/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Type object implementation */

#include <python/std.h>
#include <python/object.h>

#include <python/stringobject.h>

static void type_print(v, fp, flags)struct py_type* v;
									FILE* fp;
									int flags;
{
	(void) flags;

	fprintf(fp, "<type '%s'>", v->name);
}

static struct py_object* type_repr(v)struct py_type* v;
{
	char buf[100];
	sprintf(buf, "<type '%.80s'>", v->name);
	return py_string_new(buf);
}

struct py_type py_type_type = {
		PY_OB_SEQ_INIT(&py_type_type)
		0,                      /* Number of items for varobject */
		"type",                       /* Name of this type */
		sizeof(struct py_type),     /* Basic object size */
		0,                      /* Item size for varobject */
		0,                      /*dealloc*/
		type_print,             /*print*/
		0,                      /*get_attr*/
		0,                      /*set_attr*/
		0,                      /*cmp*/
		type_repr,              /*repr*/
		0, 0, 0
};
