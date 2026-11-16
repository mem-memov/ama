/**
 * @file
 * @brief Allocate and commit one exact source chunk without losing prior storage on failure.
 *
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) preserves original
 * bytes. [COMP-04](../../../../spec/implementation/compiler.md#comp-04) rejects
 * partial source publication; private growth owns both copies until commit.
 */
#include <string.h>
#include "buffer.h"

/** Plan before touching chunk bytes. Copy aliasing chunks before releasing an old allocation;
 * after successful allocation/copy no fallible operation remains before commit.
 * Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
SourceStatus source_buffer_append(SourceAllocator allocator, SourceBuffer *buffer,
    const unsigned char *chunk, size_t length) {
    if (buffer == NULL || (chunk == NULL && length != 0)) { return SOURCE_INVALID_ARGUMENT; }
    SourceBufferPlan plan;
    SourceStatus status = source_buffer_plan(buffer->length, buffer->capacity, length, &plan);
    if (status != SOURCE_OK) { return status; }
    bool grow = plan.capacity != buffer->capacity;
    unsigned char *destination = buffer->bytes;
    if (grow) {
        destination = allocator.allocate(allocator.context, plan.capacity);
        if (destination == NULL) { return SOURCE_OUT_OF_MEMORY; }
        if (buffer->length != 0) { memcpy(destination, buffer->bytes, buffer->length); }
    }
    if (length != 0) { memmove(destination + buffer->length, chunk, length); }
    if (grow) {
        allocator.release(allocator.context, buffer->bytes);
        buffer->bytes = destination;
        buffer->capacity = plan.capacity;
    }
    buffer->length = plan.length;
    return SOURCE_OK;
}
