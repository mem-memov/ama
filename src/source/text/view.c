/**
 * @file
 * @brief Expose immutable source views and validate half-open byte spans.
 *
 * [NAME-03](../../../spec/implementation/compiler.md#name-03)
 * Borrow original records until their manager is destroyed; never normalize bytes.
 */
#include "internal.h"

const char *source_path(const SourceText *source) {
    return source->path;
}

const unsigned char *source_bytes(const SourceText *source) {
    return source->bytes;
}

size_t source_length(const SourceText *source) {
    return source->length;
}

bool source_span_valid(SourceSpan span) {
    return span.source != NULL && span.start <= span.end
        && span.end <= source_length(span.source);
}
