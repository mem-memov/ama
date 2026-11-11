#ifndef AMA_SOURCE_TEXT_INTERNAL_H
#define AMA_SOURCE_TEXT_INTERNAL_H

#include "source.h"

struct SourceText {
    SourceText *next;
    char *path;
    unsigned char *bytes;
    size_t length;
    size_t *line_starts;
    size_t line_count;
};

void source_text_release(SourceAllocator allocator, SourceText *source);
SourceStatus source_text_lines(SourceAllocator allocator, SourceText *source);

#endif
