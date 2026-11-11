#ifndef AMA_SOURCE_TEXT_H
#define AMA_SOURCE_TEXT_H

#include <stdbool.h>
#include <stddef.h>
#include "source/memory/allocator.h"

typedef enum {
    SOURCE_OK,
    SOURCE_OUT_OF_MEMORY,
    SOURCE_LIMIT,
    SOURCE_IO_ERROR,
    SOURCE_INVALID_ARGUMENT
} SourceStatus;

typedef struct SourceText SourceText;
typedef struct {
    SourceAllocator allocator;
    SourceText *first;
} SourceManager;

typedef struct {
    const SourceText *source;
    size_t start;
    size_t end;
} SourceSpan;

typedef struct {
    size_t line;
    size_t column;
    size_t line_start;
    size_t line_end;
} SourcePosition;

/* All borrowed views and spans survive additions, until manager destruction.
 * Add copies bytes verbatim, including NUL/CRLF; UTF-8 validation is separate.
 * Failure publishes no source and leaves the output pointer unchanged. */
void source_manager_init(SourceManager *manager, SourceAllocator allocator);
void source_manager_destroy(SourceManager *manager);
SourceStatus source_manager_add(SourceManager *manager, const char *path,
    const unsigned char *bytes, size_t length, const SourceText **output);
SourceStatus source_manager_load(SourceManager *manager, const char *path,
    const SourceText **output);
const char *source_path(const SourceText *source);
const unsigned char *source_bytes(const SourceText *source);
size_t source_length(const SourceText *source);
bool source_span_valid(SourceSpan span);
bool source_position(const SourceText *source, size_t offset, SourcePosition *output);

#endif
