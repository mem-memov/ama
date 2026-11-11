#include <stdint.h>
#include "internal.h"

SourceStatus source_text_lines(SourceAllocator allocator, SourceText *source) {
    size_t count = 1;
    for (size_t offset = 0; offset < source->length; offset++) {
        if (source->bytes[offset] == '\n') {
            count++;
        }
    }
    if (count > SIZE_MAX / sizeof(size_t)) {
        return SOURCE_LIMIT;
    }
    source->line_starts = allocator.allocate(allocator.context, count * sizeof(size_t));
    if (source->line_starts == NULL) {
        return SOURCE_OUT_OF_MEMORY;
    }
    source->line_count = count;
    source->line_starts[0] = 0;
    size_t line = 1;
    for (size_t offset = 0; offset < source->length; offset++) {
        if (source->bytes[offset] == '\n') {
            source->line_starts[line++] = offset + 1;
        }
    }
    return SOURCE_OK;
}

bool source_position(const SourceText *source, size_t offset, SourcePosition *output) {
    if (source == NULL || output == NULL || offset > source->length) {
        return false;
    }
    size_t lower = 0;
    size_t upper = source->line_count;
    while (lower + 1 < upper) {
        size_t middle = lower + (upper - lower) / 2;
        if (source->line_starts[middle] <= offset) {
            lower = middle;
        } else {
            upper = middle;
        }
    }
    size_t start = source->line_starts[lower];
    size_t end = lower + 1 < source->line_count
        ? source->line_starts[lower + 1] - 1 : source->length;
    if (lower + 1 < source->line_count && end > start
        && source->bytes[end - 1] == '\r') {
        end--;
    }
    *output = (SourcePosition){lower + 1, offset - start + 1, start, end};
    return true;
}
