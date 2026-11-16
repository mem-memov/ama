/**
 * @file
 * @brief Read original file bytes with explicit temporary-buffer and stream ownership.
 *
 * [SRC-01](../../../spec/language/source-and-names.md#src-01)
 * Load physical sources without changing bytes or guessing logical module identity.
 */
#include <stdio.h>
#include "buffer/buffer.h"

/** Read finite binary chunks without interpreting bytes. Caller retains buffer ownership and stream closure on every status.
 * Component contract: [SRC-01](../../../spec/language/source-and-names.md#src-01). */
static SourceStatus read_stream(FILE *stream, SourceAllocator allocator,
    SourceBuffer *buffer) {
    unsigned char chunk[4096];
    size_t length;
    while ((length = fread(chunk, 1, sizeof(chunk), stream)) != 0) {
        SourceStatus status = source_buffer_append(allocator, buffer, chunk, length);
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
