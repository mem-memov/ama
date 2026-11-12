/**
 * @file
 * @brief Read original file bytes with explicit temporary-buffer and stream ownership.
 *
 * [SRC-01](../../../spec/language/source-and-names.md#src-01)
 * Load physical sources without changing bytes or guessing logical module identity.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "source/text/source.h"

/** Private growable binary read buffer; caller releases it even after failed read/close. */
typedef struct {
    unsigned char *bytes;
    size_t length;
    size_t capacity;
} SourceBuffer;

/** Grow private temporary bytes geometrically after checked addition. Allocate/copy before releasing old storage; failure preserves the old buffer.
 */
static SourceStatus append_chunk(SourceAllocator allocator, SourceBuffer *buffer,
    const unsigned char *chunk, size_t length) {
    if (length > SIZE_MAX - buffer->length) {
        return SOURCE_LIMIT;
    }
    size_t needed = buffer->length + length;
    if (needed > buffer->capacity) {
        size_t capacity = needed > SIZE_MAX / 2 ? needed : needed * 2;
        unsigned char *replacement = allocator.allocate(allocator.context, capacity);
        if (replacement == NULL) {
            return SOURCE_OUT_OF_MEMORY;
        }
        if (buffer->length != 0) {
            memcpy(replacement, buffer->bytes, buffer->length);
        }
        allocator.release(allocator.context, buffer->bytes);
        buffer->bytes = replacement;
        buffer->capacity = capacity;
    }
    if (length != 0) {
        memcpy(buffer->bytes + buffer->length, chunk, length);
    }
    buffer->length = needed;
    return SOURCE_OK;
}

/** Read finite binary chunks without interpreting bytes. Caller retains buffer ownership and stream closure on every status.
 */
static SourceStatus read_stream(FILE *stream, SourceAllocator allocator,
    SourceBuffer *buffer) {
    unsigned char chunk[4096];
    size_t length;
    while ((length = fread(chunk, 1, sizeof(chunk), stream)) != 0) {
        SourceStatus status = append_chunk(allocator, buffer, chunk, length);
        if (status != SOURCE_OK) {
            return status;
        }
    }
    return ferror(stream) ? SOURCE_IO_ERROR : SOURCE_OK;
}

SourceStatus source_manager_load(SourceManager *manager, const char *path,
    const SourceText **output) {
    if (path == NULL || output == NULL) {
        return SOURCE_INVALID_ARGUMENT;
    }
    FILE *stream = fopen(path, "rb");
    if (stream == NULL) {
        return SOURCE_IO_ERROR;
    }
    SourceBuffer buffer = {0};
    SourceStatus status = read_stream(stream, manager->allocator, &buffer);
    if (fclose(stream) != 0 && status == SOURCE_OK) {
        status = SOURCE_IO_ERROR;
    }
    if (status == SOURCE_OK) {
        status = source_manager_add(manager, path, buffer.bytes, buffer.length, output);
    }
    manager->allocator.release(manager->allocator.context, buffer.bytes);
    return status;
}
