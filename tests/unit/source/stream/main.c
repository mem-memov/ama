/** @file
 * @brief Verify arbitrary borrowed stream bytes, EOF, read failure and every growth refusal.
 * @see [T273 contract](../../../../docs/implementation/source-protocol.md)
 */
#include <stdio.h>
#include "../common.c"
#include "source/io/buffer/buffer.h"
/** The fixture stream contains independently calculated bytes over several chunks. */
static FILE *input(void) {
    FILE *stream = tmpfile(); assert(stream);
    for (size_t index = 0; index < 10000; index++) { assert(fputc((int)(index % 256), stream) != EOF); }
    assert(fflush(stream) == 0 && fseek(stream, 0, SEEK_SET) == 0); return stream;
}
/** Prefix appending, embedded NUL/invalid UTF-8 and EOF preserve exact bytes and borrowed stream ownership. */
static void it_reads_complete_binary_stream_without_owning_or_rewinding_it(void) {
    setup_test(); FILE *stream = input(); SourceBuffer buffer = {0};
    assert(source_buffer_append(sources.allocator, &buffer, (const unsigned char *)"pre", 3) == SOURCE_OK);
    assert(source_buffer_read(sources.allocator, stream, &buffer) == SOURCE_OK && buffer.length == 10003);
    assert(!memcmp(buffer.bytes, "pre", 3));
    for (size_t index = 0; index < 10000; index++) { assert(buffer.bytes[index + 3] == index % 256); }
    assert(source_buffer_read(sources.allocator, stream, &buffer) == SOURCE_OK && buffer.length == 10003);
    assert(fseek(stream, 0, SEEK_SET) == 0 && fclose(stream) == 0);
    sources.allocator.release(sources.allocator.context, buffer.bytes); destroy_test();
}
/** Read errors never masquerade as EOF, and invalid arguments publish nothing. */
static void it_distinguishes_read_failure_from_empty_eof(void) {
    setup_test(); FILE *stream = fopen("generated/write-only", "wb"); assert(stream);
    SourceBuffer buffer = {0};
    assert(source_buffer_read(sources.allocator, stream, &buffer) == SOURCE_IO_ERROR && !buffer.bytes);
    assert(source_buffer_read(sources.allocator, NULL, &buffer) == SOURCE_INVALID_ARGUMENT);
    assert(source_buffer_read(sources.allocator, stream, NULL) == SOURCE_INVALID_ARGUMENT);
    assert(fclose(stream) == 0); destroy_test();
}
/** Every failed growth preserves exactly the previously captured complete chunks. */
static void it_retires_each_allocation_failure_without_fabricating_a_complete_stream(void) {
    setup_test(); FILE *stream = input(); SourceBuffer buffer = {0};
    size_t before = allocation_count;
    assert(source_buffer_read(sources.allocator, stream, &buffer) == SOURCE_OK);
    size_t count = allocation_count - before;
    sources.allocator.release(sources.allocator.context, buffer.bytes); assert(fclose(stream) == 0); destroy_test();
    assert(count);
    for (size_t fault = 1; fault <= count; fault++) {
        setup_test(); stream = input(); buffer = (SourceBuffer){0}; failure_at = fault;
        assert(source_buffer_read(sources.allocator, stream, &buffer) == SOURCE_OUT_OF_MEMORY);
        assert(buffer.length < 10000);
        for (size_t index = 0; index < buffer.length; index++) { assert(buffer.bytes[index] == index % 256); }
        sources.allocator.release(sources.allocator.context, buffer.bytes); assert(fclose(stream) == 0); destroy_test();
    }
}
/** Execute stream lifetime and failure contracts explicitly. */
void run(void) {
    it_reads_complete_binary_stream_without_owning_or_rewinding_it();
    it_distinguishes_read_failure_from_empty_eof();
    it_retires_each_allocation_failure_without_fabricating_a_complete_stream();
}
/** Preserve generated evidence on failure. */
int main(void) { run(); return 0; }
