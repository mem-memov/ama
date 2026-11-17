/**
 * @file
 * @brief Scan one maximal complete Unicode name without consuming surrounding punctuation or separators.
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) makes every word
 * obey the pinned scalar/internal-hyphen profile and forbids declaration-dependent shorter splits.
 */
#include "tokens.h"
#include "source/unicode/unicode.h"

bool lex_name(SourceSpan text, size_t *cursor, SourceSpan *name) {
    if (!source_span_valid(text) || cursor == NULL || name == NULL
        || *cursor < text.start || *cursor >= text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    size_t start = *cursor;
    size_t stop = start + unicode_word_prefix(bytes + start, text.end - start);
    if (stop == start) { return false; }
    while (stop < text.end && bytes[stop] == ' ') {
        size_t next = stop + 1;
        size_t length = unicode_word_prefix(bytes + next, text.end - next);
        if (length == 0) { break; }
        stop = next + length;
    }
    *name = (SourceSpan){text.source, start, stop};
    *cursor = stop;
    return true;
}
