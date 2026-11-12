/**
 * @file
 * @brief Locate retained positional/named entry-literal boundaries without resolving names.
 *
 * [CALL-07](../../../spec/language/calls-and-results.md#call-07)
 * Preserve literal bytes after a recognized marker; exact one-space/multiword recognition remains T022/T025.
 */
#include "../layout/internal.h"
#include "source/unicode/unicode.h"

/** Recognize retained positional or XID-word label structure before an input marker. Return end for nonbinding text; multiword/exact separator changes remain T022.
 */
static size_t binding_marker(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = text.start;
    if (cursor < text.end && bytes[cursor] == '<') {
        return cursor;
    }
    while (cursor < text.end && bytes[cursor] != ' ' && bytes[cursor] != '&'
        && bytes[cursor] != '[' && bytes[cursor] != '<') {
        cursor++;
    }
    size_t invalid;
    if (!unicode_identifier(bytes + text.start, cursor - text.start, &invalid)) {
        return text.end;
    }
    if (cursor < text.end && bytes[cursor] == '&') {
        cursor++;
    }
    if (text.end - cursor >= 2 && bytes[cursor] == '[' && bytes[cursor + 1] == ']') {
        cursor += 2;
    }
    if (cursor == text.end || bytes[cursor] != ' ') {
        return text.end;
    }
    while (cursor < text.end && bytes[cursor] == ' ') {
        cursor++;
    }
    return cursor < text.end && bytes[cursor] == '<' ? cursor : text.end;
}

size_t layout_input_literal(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = binding_marker(text);
    if (cursor == text.end || ++cursor == text.end || bytes[cursor] != ' ') {
        return text.end;
    }
    while (cursor < text.end && bytes[cursor] == ' ') {
        cursor++;
    }
    return cursor < text.end && bytes[cursor] == '|' ? cursor : text.end;
}
