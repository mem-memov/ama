/**
 * @file
 * @brief Locate exact positional or complete-name input literal boundaries before comment recognition.
 * [CALL-07](../../../spec/language/calls-and-results.md#call-07) preserves raw
 * payloads; [SYN-06](../../../spec/language/source-and-names.md#syn-06) requires
 * one separator and compact declaration suffixes without name lookup.
 */
#include <string.h>
#include "../layout/internal.h"
#include "source/unicode/unicode.h"

/** Recognize the first input marker only after an exact complete declaration label or at the operand start. Component contract: [CALL-07](../../../spec/language/calls-and-results.md#call-07), [SYN-06](../../../spec/language/source-and-names.md#syn-06). */
static size_t binding_marker(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    if (text.start < text.end && bytes[text.start] == '<') { return text.start; }
    size_t marker = text.start;
    while (marker < text.end && bytes[marker] != '<' && bytes[marker] != '#') { marker++; }
    if (marker == text.end || bytes[marker] != '<' || marker == text.start || bytes[marker - 1] != ' ') { return text.end; }
    size_t end = marker - 1;
    if (end - text.start >= 3 && memcmp(bytes + end - 3, "&[]", 3) == 0) { end -= 3; }
    else if (end - text.start >= 2 && memcmp(bytes + end - 2, "[]", 2) == 0) { end -= 2; }
    else if (end > text.start && bytes[end - 1] == '&') { end--; }
    size_t error;
    return unicode_name(bytes + text.start, end - text.start, &error) ? marker : text.end;
}
size_t layout_input_literal(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t marker = binding_marker(text);
    return text.end - marker >= 3 && bytes[marker + 1] == ' ' && bytes[marker + 2] == '|'
        ? marker + 2 : text.end;
}
