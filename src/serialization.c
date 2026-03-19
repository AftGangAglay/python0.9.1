/*
* Copyright 1991 by Stichting Mathematisch Centrum
 * See `LICENCE' for more information.
 */

#include <python/serialization.h>
#include <python/object.h>

#include <asys/memory.h>

static const enum py_type py_global_redirect_type = PY_TYPE_MAX;

enum asys_result py_object_serialize(
		struct py_object* object, struct asys_stream* stream,
		enum py_serialization_scheme scheme) {

	struct py_serialization_context ctx = { 0 };

	ctx.stream = stream;
	ctx.scheme = scheme;

	return py_serialization_context_write_object(&ctx, object);
}

enum asys_result py_serialization_context_write_object(
		struct py_serialization_context* ctx, struct py_object* object) {

	enum asys_result result;
	py_serialize_t serialize;
	asys_size_t i;

	for(i = 0; i < ctx->count; ++i) {
		struct py_serialized_object* serialized = &ctx->serialized[i];

		if(serialized->object == object) {
			result = asys_stream_write(
					ctx->stream, 0, &py_global_redirect_type,
                    sizeof(enum py_type));

			if(result) return result;

			return asys_stream_write(
					ctx->stream, 0, &serialized->offset, sizeof(asys_size_t));
		}
	}

	ctx->serialized = asys_memory_reallocate_safe(
			ctx->serialized,
			++ctx->count * sizeof(struct py_serialized_object));

	if(!ctx->serialized) return ASYS_RESULT_OOM;

	result = asys_stream_tell(
			ctx->stream, &ctx->serialized[ctx->count - 1].offset);

	if(result) return result;

	result = asys_stream_write(
			ctx->stream, 0, &object->type, sizeof(enum py_type));

	if(result) return result;

	serialize = py_types[object->type].serialize;
	if(serialize) {
		serialize(object, ctx);
	}

	return ASYS_RESULT_OK;
}

enum asys_result py_object_deserialize(
		struct py_object** object, struct asys_stream* stream,
		enum py_serialization_scheme scheme) {

	struct py_serialization_context ctx = { 0 };

	ctx.stream = stream;
	ctx.scheme = scheme;

	return py_serialization_context_read_object(&ctx, object);
}

enum asys_result py_serialization_context_read_object(
		struct py_serialization_context* context, struct py_object** object) {

    (void) context;
	(void) object;

	return ASYS_RESULT_OK;
}
