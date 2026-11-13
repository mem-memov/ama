/**
 * @file
 * @brief Validate UTF-8 and split original source into owned physical line records.
 *
 * [NAME-03](../../../spec/implementation/compiler.md#name-03)
 * Retain original spans and CRLF/LF/no-ending distinctions for diagnostics and literal payloads.
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) prohibits TAB and
 * lone CR before activation checks or literal interpretation.
 */
#include "internal.h"

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

/** Allocate/classify one line privately, including CRLF exclusion, then append. Allocation failure leaves prior nodes and the exact failed site owned.
 */
static LayoutStatus append_line(SourceLayout *layout, size_t start, size_t end,
    bool newline, bool literal_payloads) {
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
    layout_classify(line, literal_payloads);
    if (layout->last == NULL) {
        layout->first = line;
    } else {
        layout->last->next = line;
    }
    layout->last = line;
    return LAYOUT_OK;
}

/** Validate the complete finite byte input before splitting at LF; preserve original bytes and create no fictitious final line after a terminator.
 */
static LayoutStatus read_source(SourceLayout *layout, const SourceText *source,
    bool literal_payloads) {
    layout->source = source;
    size_t length = source_length(source);
    const unsigned char *bytes = source_bytes(source);
    LayoutStatus checked = layout_validate_source(layout);
    if (checked != LAYOUT_OK) { return checked; }
    size_t start = 0;
    while (start < length) {
        size_t end = start;
        while (end < length && bytes[end] != '\n') {
            end++;
        }
        LayoutStatus status = append_line(layout, start, end, end < length, literal_payloads);
        if (status != LAYOUT_OK) {
            return status;
        }
        start = end < length ? end + 1 : length;
    }
    return LAYOUT_OK;
}

LayoutStatus layout_read(SourceLayout *layout, const SourceText *source) {
    return read_source(layout, source, true);
}

LayoutStatus layout_read_abi(SourceLayout *layout, const SourceText *source) {
    return read_source(layout, source, false);
}
