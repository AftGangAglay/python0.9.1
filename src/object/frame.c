/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Frame object implementation */

#include <python/compile.h>
#include <python/opcode.h>

#include <python/object/frame.h>
#include <python/object/dict.h>

#include <asys/memory.h>

struct py_frame* py_frame_new(
		struct py_frame* back, struct py_code* code, struct py_object* globals,
		struct py_object* locals, unsigned nvalues, unsigned nblocks) {

	struct py_frame* f;

	if(!(f = py_object_new(PY_TYPE_FRAME))) return 0;

	f->back = py_object_incref(back);
	f->code = py_object_incref(code);
	f->globals = py_object_incref(globals);
	f->locals = py_object_incref(locals);

	f->valuestack = asys_memory_allocate_zero(
			nvalues + 1, sizeof(struct py_object*));

	if(!f->valuestack) goto cleanup;

	f->blockstack = asys_memory_allocate_zero(
			nblocks + 1, sizeof(struct py_block));

	if(!f->blockstack) goto cleanup;

	f->nblocks = nblocks;
	f->iblock = 0;

	return f;

	cleanup: {
		py_object_decref(f);
		return 0;
	}
}

/* Block management */

void py_block_setup(
		struct py_frame* f, enum py_opcode type, unsigned handler,
		unsigned level) {

	struct py_block* b = &f->blockstack[f->iblock++];

	b->type = type;
	b->level = level;
	b->handler = handler;
}

struct py_block* py_block_pop(struct py_frame* f) {
	return &f->blockstack[--f->iblock];
}

void py_frame_dealloc(struct py_object* op) {
	struct py_frame* f = (void*) op;

	py_object_decref(f->back);
	py_object_decref(f->code);
	py_object_decref(f->globals);
	py_object_decref(f->locals);

	asys_memory_free(f->valuestack);
	asys_memory_free(f->blockstack);

	asys_memory_free(op);
}
