/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Class object implementation */

#include <python/std.h>
#include <python/structmember.h>
#include <python/errors.h>

#include <python/object.h>
#include <python/classobject.h>
#include <python/dictobject.h>
#include <python/tupleobject.h>
#include <python/funcobject.h>

struct py_class {
	PY_OB_SEQ
	struct py_object* bases; /* A tuple */
	struct py_object* methods; /* A dictionary */
};
struct py_object* py_class_new(
		struct py_object* bases, struct py_object* methods) {

	struct py_class* op;

	op = py_object_new(&py_class_type);
	if(op == NULL) return NULL;

	if(bases != NULL) PY_INCREF(bases);

	op->bases = bases;
	PY_INCREF(methods);
	op->methods = methods;

	return (struct py_object*) op;
}

/* Class methods */

static void py_class_dealloc(struct py_object* op) {
	struct py_class* cls;

	cls = (struct py_class*) op;

	if(cls->bases != NULL) PY_DECREF(cls->bases);
	PY_DECREF(cls->methods);

	free(op);
}

static struct py_object* py_class_get_attr(
		struct py_object* op, const char* name) {

	struct py_object* v;
	struct py_class* cls;

	cls = (struct py_class*) op;
	v = py_dict_lookup(cls->methods, name);

	if(v != NULL) {
		PY_INCREF(v);
		return v;
	}

	if(cls->bases != NULL) {
		int n = py_tuple_size(cls->bases);
		int i;

		for(i = 0; i < n; i++) {
			v = py_class_get_attr(py_tuple_get(cls->bases, i), name);
			if(v != NULL) return v;

			py_error_clear();
		}
	}

	py_error_set_string(py_name_error, name);
	return NULL;
}

struct py_type py_class_type = {
		PY_OB_SEQ_INIT(&py_type_type)
		0, "class", sizeof(struct py_class), 0,
		py_class_dealloc, /* dealloc */
		0, /* print */
		py_class_get_attr, /* get_attr */
		0, /* set_attr */
		0, /* cmp */
		0, /* repr */
		0, /* numbermethods */
		0, /* sequencemethods */
		0, /* mappingmethods */
};

/* We're not done yet: next, we define class member objects... */
typedef struct {
	PY_OB_SEQ
	struct py_class* cm_class;      /* The class object */
	struct py_object* cm_attr;       /* A dictionary */
} classmemberobject;

struct py_object* py_classmember_new(class)struct py_object* class;
{
	classmemberobject* cm;
	if(!py_is_class(class)) {
		py_error_set_badcall();
		return NULL;
	}
	cm = py_object_new(&py_class_member_type);
	if(cm == NULL) {
		return NULL;
	}
	PY_INCREF(class);
	cm->cm_class = (struct py_class*) class;
	cm->cm_attr = py_dict_new();
	if(cm->cm_attr == NULL) {
		PY_DECREF(cm);
		return NULL;
	}
	return (struct py_object*) cm;
}

/* Class member methods */

static void classmember_dealloc(cm)classmemberobject* cm;
{
	PY_DECREF(cm->cm_class);
	if(cm->cm_attr != NULL)
		PY_DECREF(cm->cm_attr);
	free(cm);
}

static struct py_object* classmember_getattr(cm, name)classmemberobject* cm;
													  char* name;
{
	struct py_object* v = py_dict_lookup(cm->cm_attr, name);
	if(v != NULL) {
		PY_INCREF(v);
		return v;
	}
	v = py_class_get_attr((struct py_object*) cm->cm_class, name);
	if(v == NULL) {
		return v;
	} /* py_class_get_attr() has set the error */
	if(py_is_func(v)) {
		struct py_object* w = py_classmethod_new(v, (struct py_object*) cm);
		PY_DECREF(v);
		return w;
	}
	PY_DECREF(v);
	py_error_set_string(py_name_error, name);
	return NULL;
}

static int classmember_setattr(cm, name, v)classmemberobject* cm;
										   char* name;
										   struct py_object* v;
{
	if(v == NULL) {
		return py_dict_remove(cm->cm_attr, name);
	}
	else {
		return py_dict_insert(cm->cm_attr, name, v);
	}
}

struct py_type py_class_member_type = {
		PY_OB_SEQ_INIT(&py_type_type) 0, "class member",
		sizeof(classmemberobject), 0, classmember_dealloc,    /*dealloc*/
		0,                      /*print*/
		classmember_getattr,    /*get_attr*/
		classmember_setattr,    /*set_attr*/
		0,                      /*cmp*/
		0,                      /*repr*/
		0,                      /*numbermethods*/
		0,                      /*sequencemethods*/
		0,                      /*mappingmethods*/
};


/* And finally, here are class method objects */

/* (Really methods of class members) */

typedef struct {
	PY_OB_SEQ
	struct py_object* cm_func;       /* The method function */
	struct py_object* cm_self;       /* The object to which this applies */
} classmethodobject;

struct py_object* py_classmethod_new(func, self)struct py_object* func;
												struct py_object* self;
{
	classmethodobject* cm;
	if(!py_is_func(func)) {
		py_error_set_badcall();
		return NULL;
	}
	cm = py_object_new(&py_class_method_type);
	if(cm == NULL) {
		return NULL;
	}
	PY_INCREF(func);
	cm->cm_func = func;
	PY_INCREF(self);
	cm->cm_self = self;
	return (struct py_object*) cm;
}

struct py_object* py_classmethod_get_func(cm)struct py_object* cm;
{
	if(!py_is_classmethod(cm)) {
		py_error_set_badcall();
		return NULL;
	}
	return ((classmethodobject*) cm)->cm_func;
}

struct py_object* py_classmethod_get_self(cm)struct py_object* cm;
{
	if(!py_is_classmethod(cm)) {
		py_error_set_badcall();
		return NULL;
	}
	return ((classmethodobject*) cm)->cm_self;
}

/* Class method methods */

#define OFF(x) offsetof(classmethodobject, x)

static struct py_memberlist classmethod_memberlist[] = {
		{ "cm_func", PY_TYPE_OBJECT, OFF(cm_func), PY_READWRITE },
		{ "cm_self", PY_TYPE_OBJECT, OFF(cm_self), PY_READWRITE },
		{ NULL,      0, 0,                         0 }  /* Sentinel */
};

static struct py_object* classmethod_getattr(cm, name)classmethodobject* cm;
													  char* name;
{
	return py_memberlist_get((char*) cm, classmethod_memberlist, name);
}

static void classmethod_dealloc(cm)classmethodobject* cm;
{
	PY_DECREF(cm->cm_func);
	PY_DECREF(cm->cm_self);
	free(cm);
}

struct py_type py_class_method_type = {
		PY_OB_SEQ_INIT(&py_type_type) 0, "class method",
		sizeof(classmethodobject), 0, classmethod_dealloc,    /*dealloc*/
		0,                      /*print*/
		classmethod_getattr,    /*get_attr*/
		0,                      /*set_attr*/
		0,                      /*cmp*/
		0,                      /*repr*/
		0,                      /*numbermethods*/
		0,                      /*sequencemethods*/
		0,                      /*mappingmethods*/
};
