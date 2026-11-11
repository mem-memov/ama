#include "internal.h"
#include "source/unicode/unicode.h"

void layout_init(SourceLayout *layout, SourceAllocator allocator) {
    *layout = (SourceLayout){0};
    source_arena_init(&layout->arena, allocator);
}

void layout_destroy(SourceLayout *layout) {
    source_arena_destroy(&layout->arena);
    layout->first = NULL;
    layout->last = NULL;
}

LayoutStatus layout_failure(SourceLayout *layout, LayoutStatus status,
    SourceSpan span, const char *message, const char *expected) {
    layout->diagnostic = (Diagnostic){0};
    layout->diagnostic.code = status == LAYOUT_OUT_OF_MEMORY
        ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_SYNTAX;
    layout->diagnostic.message = message;
    layout->diagnostic.expected = expected;
    layout->diagnostic.primary.span = span;
    return status;
}

static LayoutStatus append_line(SourceLayout *layout, size_t start, size_t end, bool newline) {
    const unsigned char *bytes = source_bytes(layout->source);
    bool crlf = newline && end > start && bytes[end - 1] == '\r';
    LayoutLine *line = source_arena_allocate(&layout->arena, sizeof(*line));
    if (line == NULL) {
        return layout_failure(layout, LAYOUT_OUT_OF_MEMORY,
            (SourceSpan){layout->source, start, start}, "Cannot allocate source lines", NULL);
    }
    *line = (LayoutLine){0};
    line->physical = (SourceSpan){layout->source, start, end - crlf};
    line->ending = crlf ? ENDING_CRLF : newline ? ENDING_LF : ENDING_NONE;
    layout_classify(line);
    if (layout->last == NULL) {
        layout->first = line;
    } else {
        layout->last->next = line;
    }
    layout->last = line;
    return LAYOUT_OK;
}

LayoutStatus layout_read(SourceLayout *layout, const SourceText *source) {
    layout->source = source;
    size_t length = source_length(source);
    const unsigned char *bytes = source_bytes(source);
    size_t error;
    if (!utf8_validate(bytes, length, &error)) {
        LayoutStatus status = layout_failure(layout, LAYOUT_INVALID,
            (SourceSpan){source, error, error + 1}, "Source is not valid UTF-8", "well-formed UTF-8");
        layout->diagnostic.code = DIAGNOSTIC_INVALID_UTF8;
        return status;
    }
    size_t start = 0;
    while (start < length) {
        size_t end = start;
        while (end < length && bytes[end] != '\n') {
            end++;
        }
        LayoutStatus status = append_line(layout, start, end, end < length);
        if (status != LAYOUT_OK) {
            return status;
        }
        start = end < length ? end + 1 : length;
    }
    return LAYOUT_OK;
}
