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
