/** @file
 * @brief Capture complete borrowed stream bytes without confusing failure with EOF.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and managed I/O failure; [SRC-03](../../../../spec/language/source-and-names.md#src-03)
 * defers encoding/role interpretation to the input consumer.
 */
#include "buffer.h"
SourceStatus source_buffer_read(SourceAllocator allocator, FILE *stream, SourceBuffer *buffer) {
    if (!stream || !buffer) { return SOURCE_INVALID_ARGUMENT; }
    unsigned char chunk[4096];
    size_t length;
    while ((length = fread(chunk, 1, sizeof(chunk), stream)) != 0) {
        SourceStatus status = source_buffer_append(allocator, buffer, chunk, length);
        if (status != SOURCE_OK) { return status; }
    }
    return ferror(stream) ? SOURCE_IO_ERROR : SOURCE_OK;
}
