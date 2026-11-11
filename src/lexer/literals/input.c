#include "../layout/internal.h"
#include "source/unicode/unicode.h"

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
