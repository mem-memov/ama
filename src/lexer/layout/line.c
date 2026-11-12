/**
 * @file
 * @brief Classify physical indentation, comment code and untouched literal payload boundaries.
 *
 * [LIT-01](../../../spec/language/declarations.md#lit-01)
 * Payload spans retain original bytes; exact current spacing/comment placement remains T022 work.
 */
#include <string.h>
#include "internal.h"

void layout_classify(LayoutLine *line, bool literal_payloads) {
    const unsigned char *bytes = source_bytes(line->physical.source);
    size_t start = line->physical.start;
    size_t end = line->physical.end;
    while (start < end && bytes[start] == ' ') {
        start++;
    }
    line->indentation = start - line->physical.start;
    line->kind = start == end ? LINE_BLANK : bytes[start] == '#' ? LINE_COMMENT
        : literal_payloads && bytes[start] == '|' ? LINE_LITERAL : LINE_CODE;
    size_t literal = line->kind == LINE_LITERAL ? start
        : literal_payloads && line->kind == LINE_CODE
            ? layout_input_literal((SourceSpan){line->physical.source, start, end}) : end;
    if (literal < end) {
        line->kind = literal == start ? LINE_LITERAL : LINE_INPUT_LITERAL;
        line->payload = (SourceSpan){line->physical.source, literal + 1, end};
        line->code = (SourceSpan){line->physical.source, start, literal + 1};
        return;
    }
    line->payload = (SourceSpan){line->physical.source, end, end};
    size_t code_end = start;
    while (code_end < end && bytes[code_end] != '#') {
        code_end++;
    }
    while (code_end > start && bytes[code_end - 1] == ' ') {
        code_end--;
    }
    line->code = (SourceSpan){line->physical.source, start, code_end};
}

bool layout_text_is(SourceSpan span, const char *text) {
    size_t length = strlen(text);
    return span.source != NULL && span.end >= span.start
        && span.end - span.start == length
        && memcmp(source_bytes(span.source) + span.start, text, length) == 0;
}
