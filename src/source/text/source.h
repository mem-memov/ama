/**
 * @file
 * @brief Own immutable source bytes and expose bounded borrowed spans.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * Preserve complete original bytes; lexical interpretation remains a separate stage.
 */
#ifndef AMA_SOURCE_TEXT_H
#define AMA_SOURCE_TEXT_H

#include <stdbool.h>
#include <stddef.h>
#include "source/memory/allocator.h"

/** Source construction outcomes; failure never publishes a new record. */
typedef enum {
    SOURCE_OK,
    SOURCE_OUT_OF_MEMORY,
    SOURCE_LIMIT,
    SOURCE_IO_ERROR,
    SOURCE_INVALID_ARGUMENT
} SourceStatus;

typedef struct SourceText SourceText;
/** Manager owns records and borrows allocator authority until destruction. */
typedef struct {
    SourceAllocator allocator;
    SourceText *first;
} SourceManager;

/** Borrowed half-open byte range in a live immutable source. */
typedef struct {
    const SourceText *source;
    size_t start;
    size_t end;
} SourceSpan;

/** Physical one-based line/byte column and newline-free excerpt bounds. */
typedef struct {
    size_t line;
    size_t column;
    size_t line_start;
    size_t line_end;
} SourcePosition;

/* All borrowed views and spans survive additions, until manager destruction.
 * Add copies bytes verbatim, including NUL/CRLF; UTF-8 validation is separate.
 * Failure publishes no source and leaves the output pointer unchanged. */
/** Start an empty owner with borrowed allocator callbacks valid until destruction.
 */
void source_manager_init(SourceManager *manager, SourceAllocator allocator);
/** Release all source/path/byte/line allocations. Every previously borrowed view/span expires.
 */
void source_manager_destroy(SourceManager *manager);
/** Copy path and exact bytes, then publish one complete stable record. Inputs need survive only this call.
 * @param[out] output Changes only on SOURCE_OK; other manager records remain valid on failure.
 * @return Invalid argument, checked-size/allocation failure, or success; UTF-8 validation is separate.
 */
SourceStatus source_manager_add(SourceManager *manager, const char *path,
    const unsigned char *bytes, size_t length, const SourceText **output);
/** Read a borrowed path in binary mode and close the private stream on every path.
 * @param[out] output Changes only after reading and copying a complete record.
 * @return Preserve the first read/allocation error; failed close also prevents publication.
 */
SourceStatus source_manager_load(SourceManager *manager, const char *path,
    const SourceText **output);
/** Borrow the original NUL-terminated path of a live source record; manager destruction invalidates it.
 */
const char *source_path(const SourceText *source);
/** Borrow exact immutable bytes, including embedded NUL; use source_length rather than strlen.
 */
const unsigned char *source_bytes(const SourceText *source);
/** Return the original byte count, excluding the convenience terminator.
 */
size_t source_length(const SourceText *source);
/** Check a borrowed half-open span against its live source length; null source is invalid.
 */
bool source_span_valid(SourceSpan span);
/** Find the greatest line-start not exceeding offset, including EOF. Columns count bytes.
 * @param[out] output Changes only on true; excerpts exclude LF and its preceding CR.
 * @return false for null arguments or offset past EOF; never allocate or modify source bytes.
 */
bool source_position(const SourceText *source, size_t offset, SourcePosition *output);

#endif
