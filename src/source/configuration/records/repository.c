/**
 * @file
 * @brief Validate repository namespace prefixes and relative checkout directories.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) separates a logical
 * vendor/package prefix from a physical relative directory. Dot reduction checks
 * lexical containment only; acquisition must separately check actual filesystem
 * containment and representability before accessing a checkout.
 */
#include "internal.h"
#include "source/unicode/unicode.h"

bool configuration_repository_prefix(SourceSpan text) {
    if (!source_span_valid(text) || text.end - text.start < 2) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    if (bytes[text.start] != '/') { return false; }
    size_t cursor = text.start + 1;
    while (cursor < text.end) {
        size_t end = cursor;
        while (end < text.end && bytes[end] != '/') { end++; }
        size_t error;
        if (!unicode_name(bytes + cursor, end - cursor, &error)) { return false; }
        if (end == text.end) { return true; }
        cursor = end + 1;
    }
    return false;
}

bool configuration_repository_directory(SourceSpan text) {
    if (!source_span_valid(text) || text.start == text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    if (bytes[text.start] == '/') { return false; }
    size_t depth = 0;
    size_t cursor = text.start;
    while (cursor < text.end) {
        size_t end = cursor;
        while (end < text.end && bytes[end] != '/') {
            if (bytes[end] == 0) { return false; }
            end++;
        }
        size_t length = end - cursor;
        if (length == 2 && bytes[cursor] == '.' && bytes[cursor + 1] == '.') {
            if (depth == 0) { return false; }
            depth--;
        } else if (length != 0 && !(length == 1 && bytes[cursor] == '.')) {
            depth++;
        }
        if (end == text.end) { break; }
        cursor = end + 1;
    }
    return true;
}
