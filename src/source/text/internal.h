/**
 * @file
 * @brief Declare private source-record storage and partial-construction cleanup.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Keep source ownership independent of parser/compiler models.
 */
#ifndef AMA_SOURCE_TEXT_INTERNAL_H
#define AMA_SOURCE_TEXT_INTERNAL_H

#include "source.h"

/** Private stable record owning path, exact bytes and LF line-start table. */
struct SourceText {
    SourceText *next;
    char *path;
    unsigned char *bytes;
    size_t length;
    size_t *line_starts;
    size_t line_count;
};

/** Release a possibly partial zero-initialized private record, or do nothing for null. Allocator accepts null releases.
 */
void source_text_release(SourceAllocator allocator, SourceText *source);
/** Build LF-derived line starts for a private source. Existing bytes/path stay owned for cleanup on size/allocation failure.
 */
SourceStatus source_text_lines(SourceAllocator allocator, SourceText *source);

#endif
