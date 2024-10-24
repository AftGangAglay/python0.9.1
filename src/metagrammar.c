/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

#include <python/grammar.h>

static struct py_arc arcs_0_0[3] = { { 2, 0 }, { 3, 0 }, { 4, 1 } };
static struct py_arc arcs_0_1[1] = { { 0, 1 } };

static struct py_state states_0[2] = {
		{ 3, arcs_0_0, 0, 0, 0, 0 },
		{ 1, arcs_0_1, 0, 0, 0, 0 }
};

static struct py_arc arcs_1_0[1] = { { 5, 1 } };
static struct py_arc arcs_1_1[1] = { { 6, 2 } };
static struct py_arc arcs_1_2[1] = { { 7, 3 } };
static struct py_arc arcs_1_3[1] = { { 3, 4 } };
static struct py_arc arcs_1_4[1] = { { 0, 4 } };

static struct py_state states_1[5] = {
		{ 1, arcs_1_0, 0, 0, 0, 0 },
		{ 1, arcs_1_1, 0, 0, 0, 0 },
		{ 1, arcs_1_2, 0, 0, 0, 0 },
		{ 1, arcs_1_3, 0, 0, 0, 0 },
		{ 1, arcs_1_4, 0, 0, 0, 0 }
};

static struct py_arc arcs_2_0[1] = { { 8, 1 } };
static struct py_arc arcs_2_1[2] = { { 9, 0 }, { 0, 1 } };

static struct py_state states_2[2] = {
		{ 1, arcs_2_0, 0, 0, 0, 0 },
		{ 2, arcs_2_1, 0, 0, 0, 0 }
};

static struct py_arc arcs_3_0[1] = { { 10, 1 } };
static struct py_arc arcs_3_1[2] = { { 10, 1 }, { 0,  1 } };

static struct py_state states_3[2] = {
		{ 1, arcs_3_0, 0, 0, 0, 0 },
		{ 2, arcs_3_1, 0, 0, 0, 0 }
};

static struct py_arc arcs_4_0[2] = { { 11, 1 }, { 13, 2 } };
static struct py_arc arcs_4_1[1] = { { 7, 3 } };
static struct py_arc arcs_4_2[3] = { { 14, 4 }, { 15, 4 }, { 0,  2 } };
static struct py_arc arcs_4_3[1] = { { 12, 4 } };
static struct py_arc arcs_4_4[1] = { { 0, 4 } };

static struct py_state states_4[5] = {
		{ 2, arcs_4_0, 0, 0, 0, 0 },
		{ 1, arcs_4_1, 0, 0, 0, 0 },
		{ 3, arcs_4_2, 0, 0, 0, 0 },
		{ 1, arcs_4_3, 0, 0, 0, 0 },
		{ 1, arcs_4_4, 0, 0, 0, 0 }
};

static struct py_arc arcs_5_0[3] = { { 5, 1 }, { 16, 1 }, { 17, 2 } };
static struct py_arc arcs_5_1[1] = { { 0, 1 } };
static struct py_arc arcs_5_2[1] = { { 7, 3 } };
static struct py_arc arcs_5_3[1] = { { 18, 1 } };

static struct py_state states_5[4] = {
		{ 3, arcs_5_0, 0, 0, 0, 0 },
		{ 1, arcs_5_1, 0, 0, 0, 0 },
		{ 1, arcs_5_2, 0, 0, 0, 0 },
		{ 1, arcs_5_3, 0, 0, 0, 0 }
};

static struct py_dfa dfas[6] = {
		{ 256, "MSTART", 0, 2, states_0, (py_byte_t*) "\070\000\000" },
		{ 257, "RULE", 0, 5, states_1, (py_byte_t*) "\040\000\000" },
		{ 258, "RHS", 0, 2, states_2, (py_byte_t*) "\040\010\003" },
		{ 259, "ALT", 0, 2, states_3, (py_byte_t*) "\040\010\003" },
		{ 260, "ITEM", 0, 5, states_4, (py_byte_t*) "\040\010\003" },
		{ 261, "ATOM", 0, 4, states_5, (py_byte_t*) "\040\000\003" }
};

static struct py_label labels[19] = {
		{ 0, "EMPTY" },
		{ 256, 0 },
		{ 257, 0 },
		{ 4, 0 },
		{ 0, 0 },
		{ 1, 0 },
		{ 11, 0 },
		{ 258, 0 },
		{ 259, 0 },
		{ 18, 0 },
		{ 260, 0 },
		{ 9, 0 },
		{ 10, 0 },
		{ 261, 0 },
		{ 16, 0 },
		{ 14, 0 },
		{ 3, 0 },
		{ 7, 0 },
		{ 8, 0 }
};

struct py_grammar py_meta_grammar = { 6, dfas, { 19, labels }, 256, 0 };
