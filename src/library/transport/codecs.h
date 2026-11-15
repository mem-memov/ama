/** @file
 * @brief Share exact nominal mappings among native codec emission operations.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) binds the
 * resolved export schema to its own project, never to a similar basename.
 */
#ifndef AMA_LIBRARY_TRANSPORT_CODECS_INTERNAL_H
#define AMA_LIBRARY_TRANSPORT_CODECS_INTERNAL_H
#include "internal.h"
#include "codec.h"
/** Borrow immutable validated models and one sticky output writer during generation. */
typedef struct { LibraryHeader output; const NativeScaffold *assembly; } LibraryCodecs;
/** Find one exact semantic type's registered scaffold model, or NULL. */
const ScaffoldType *library_codec_type(const LibraryCodecs *, const SemanticType *);
/** Return the exact assembly descriptor ordinal of one registered nominal type. */
size_t library_codec_type_id(const LibraryCodecs *, const SemanticType *);
/** Map an exact semantic declaration to its planned physical instance slot.
 * Constants are not instance fields; declaration ordinals are not layout slots.
 * Return SIZE_MAX when this checked assembly contains no such physical field.
 */
size_t library_codec_field_slot(const LibraryCodecs *, const SemanticSymbol *);
/** Emit one scalar's fresh native decoder and independently owned encoder. */
void library_codec_scalar(LibraryCodecs *, const LibrarySchema *);
/** Emit the scalar/list recipe builder with synchronous C input lifetime. */
void library_codec_recipes(LibraryCodecs *, const LibrarySchema *);
/** Emit independently owned list encoding with partial-prefix cleanup. */
void library_codec_list(LibraryCodecs *, const LibrarySchema *);
/** Emit export-specific shape validation and synchronous typed input recipe preparation. */
void library_codec_inputs(LibraryCodecs *, const LibraryExport *);
/** Emit typed result parameter declarations or invocation names, including leading commas. */
void library_codec_result_arguments(LibraryCodecs *, const LibraryExport *, bool);
/** Emit pure output destination admission shared by retained reads and accepted executions. */
void library_codec_result_shape(LibraryCodecs *, const LibraryExport *);
/** Emit optional retained reads with checked ownership and atomic snapshot publication. */
void library_codec_read(LibraryCodecs *, const LibraryExport *, size_t);
#endif
