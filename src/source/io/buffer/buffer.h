/**
 * @file
 * @brief Plan and append exact binary source chunks before immutable source publication.
 *
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) preserves bytes;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) forbids fabricated
 * EOF bytes/lines. One representable convenience terminator is reserved for the final owner.
 */
#ifndef AMA_SOURCE_IO_BUFFER_H
#define AMA_SOURCE_IO_BUFFER_H
#include "source/text/source.h"

/** Private growable read storage, initialized zero. Owns only bytes; caller releases
 * them through the same allocator after load succeeds or fails. length <= capacity < SIZE_MAX.
 * Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
typedef struct {
    unsigned char *bytes;
    size_t length;
    size_t capacity;
} SourceBuffer;

/** Checked post-append byte length and capacity; no storage is acquired while planning. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
typedef struct {
    size_t length;
    size_t capacity;
} SourceBufferPlan;

/** Plan exact addition, preserving current capacity when sufficient; otherwise double
 * required size, saturating once at SIZE_MAX-1. Reject invalid owner bounds or an
 * unrepresentable final source/terminator before arithmetic/output mutation.
 * Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
SourceStatus source_buffer_plan(size_t, size_t, size_t, SourceBufferPlan *);
/** Append borrowed bytes exactly, including embedded zeros and partial UTF-8 sequences.
 * NULL chunk is valid only for zero length. Chunk may alias current buffer storage:
 * copy it before releasing old storage. Failure preserves all prior buffer bytes,
 * length/capacity and ownership; no source record is published here.
 * Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
SourceStatus source_buffer_append(SourceAllocator, SourceBuffer *, const unsigned char *, size_t);
#endif
