/*
 * Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

#ifndef PY_SERIALIZATION_H
#define PY_SERIALIZATION_H

#include <asys/base.h>
#include <asys/stream.h>

enum py_serialization_scheme {
	PY_SERIALIZE_BINARY
	/* TODO: Implement AGA-style SGML serialization. */
};

struct py_serialized_object {
	struct py_object* object;
	asys_offset_t offset;
};

struct py_serialization_context {
	struct asys_stream* stream;
	enum py_serialization_scheme scheme;

	struct py_serialized_object* serialized;
	asys_size_t count;
};

typedef enum asys_result (*py_serialize_t)(
		struct py_object*, struct py_serialization_context*);

typedef enum asys_result (*py_deserialize_t)(
		struct py_object**, struct py_serialization_context*);

enum asys_result py_object_serialize(
		struct py_object*, struct asys_stream*, enum py_serialization_scheme);

enum asys_result py_serialization_context_write_object(
		struct py_serialization_context*, struct py_object*);

enum asys_result py_object_deserialize(
		struct py_object**, struct asys_stream*, enum py_serialization_scheme);

enum asys_result py_serialization_context_read_object(
		struct py_serialization_context*, struct py_object**);

#endif
