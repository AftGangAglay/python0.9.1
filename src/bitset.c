/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

/* Bitset primitives used by the parser generator */

#include <python/bitset.h>

#include <asys/memory.h>

py_bitset_t py_bitset_new(unsigned nbits) {
	return asys_memory_allocate_zero(PY_NBYTES(nbits), sizeof(py_byte_t));
}

void py_bitset_delete(py_bitset_t ss) {
	asys_memory_free(ss);
}

int py_bitset_add(py_bitset_t ss, unsigned bit) {
	unsigned byte = bit / PY_BIT;
	py_byte_t mask = PY_BIT2MASK(bit);

	if(ss[byte] & mask) return 0; /* Bit already set */
	ss[byte] |= mask;

	return 1;
}

int py_bitset_cmp(py_bitset_t ss1, py_bitset_t ss2, unsigned nbits) {
	unsigned i;

	for(i = 0; i < PY_NBYTES(nbits); ++i) {
		if(*ss1++ != *ss2++) return 0;
	}

	return 1;
}

void py_bitset_merge(py_bitset_t ss1, py_bitset_t ss2, unsigned nbits) {
	unsigned i;

	for(i = 0; i < PY_NBYTES(nbits); ++i) *ss1++ |= *ss2++;
}
