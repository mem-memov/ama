#ifndef AMA_COMMON_SOURCE_H
#define AMA_COMMON_SOURCE_H

#include <stdint.h>
#include <stdio.h>

/* Paths are borrowed; columns are one-based UTF-8 byte positions, not glyphs. */
typedef struct {
    const char* path;
    uint32_t line;
    uint32_t column;
} SourceLocation;

#define GENERATED_SOURCE_PATH "<ama-generated>"

void emit_line_directive(FILE* output, const char* path, uint32_t line);

#endif
