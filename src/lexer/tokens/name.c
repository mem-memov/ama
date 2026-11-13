/**
 * @file
 * @brief Scan one maximal complete Unicode name without consuming surrounding punctuation or separators.
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) makes every word
 * obey XID start/continue rules and forbids declaration-dependent shorter splits.
 */
#include "tokens.h"
#include "source/unicode/unicode.h"

/** Scan one bounded XID word, returning its start unchanged when no valid starting scalar exists. */
static size_t word(const unsigned char *bytes, size_t offset, size_t end) {
    size_t start = offset;
    while (offset < end) {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes + offset, end - offset, &scalar, &width)
            || !(offset == start ? unicode_identifier_start(scalar) : unicode_identifier_continue(scalar))) { break; }
        offset += width;
    }
    return offset;
}
bool lex_name(SourceSpan text, size_t *cursor, SourceSpan *name) {
    if (!source_span_valid(text) || cursor == NULL || name == NULL
        || *cursor < text.start || *cursor >= text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    size_t start = *cursor;
    size_t stop = word(bytes, start, text.end);
    if (stop == start) { return false; }
    while (stop < text.end && bytes[stop] == ' ') {
        size_t next = word(bytes, stop + 1, text.end);
        if (next == stop + 1) { break; }
        stop = next;
    }
    *name = (SourceSpan){text.source, start, stop};
    *cursor = stop;
    return true;
}
