/**
 * @file
 * @brief Recognize canonical selector pattern components without matching identities.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) admits ordinary/package-
 * alias patterns, forbids whole dot/parent components, and never normalizes a
 * wildcard away. Matching only eligible package identities belongs to T262.
 */
#include "internal.h"
#include "source/configuration/lines/internal.h"
#include "source/unicode/unicode.h"

/** Consume one nonempty pattern component, retaining all literal scalar bytes.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) preserves complete pattern components and forbids dot traversal.
 */
static bool component(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t length = text.end - text.start;
    if (length == 0 || configuration_text_is(text, ".") || configuration_text_is(text, "..")) { return false; }
    for (size_t offset = text.start; offset < text.end;) {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes + offset, text.end - offset, &scalar, &width)) { return false; }
        if (configuration_space(scalar) && (offset == text.start || offset + width == text.end)) { return false; }
        if (!unicode_identifier_continue(scalar) && scalar != '-' && scalar != '.' && scalar != ' ' && scalar != '*') {
            return false;
        }
        offset += width;
    }
    return true;
}

bool configuration_selector(SourceSpan text) {
    if (!source_span_valid(text) || text.start == text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = text.start;
    if (bytes[cursor] == '~') {
        size_t name = unicode_name_prefix(bytes + cursor + 1, text.end - cursor - 1);
        if (name == 0) { return false; }
        cursor += name + 1;
        if (cursor == text.end) { return true; }
        if (bytes[cursor] != '/') { return false; }
        cursor++;
    } else if (bytes[cursor] == '/') {
        cursor++;
    } else { return false; }
    if (cursor == text.end) { return false; }
    while (cursor < text.end) {
        size_t end = cursor;
        while (end < text.end && bytes[end] != '/') { end++; }
        if (!component((SourceSpan){text.source, cursor, end})) { return false; }
        if (end == text.end) { return true; }
        cursor = end + 1;
    }
    return false;
}
