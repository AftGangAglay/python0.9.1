/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Module definition and import interface */

#ifndef PY_IMPORT_H
#define PY_IMPORT_H

#include <python/object.h>

void py_import_init(void);
void py_import_done(void);

struct py_object* py_get_modules(void);
struct py_object* py_add_module(char*);
struct py_object* py_import_module(char*);
struct py_object* py_reload_module(struct py_object*);

#endif