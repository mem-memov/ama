/**
 * @file
 * @brief Validate complete Ama names as shared words separated by single ASCII spaces.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) applies the start
 * rule to every word and forbids trimming, repeated/edge spaces and normalization.
 * Token boundaries and path-root syntax remain separate consumers of this predicate.
 */
#include "unicode.h"

/** Validate words through the shared word predicate, preserving global byte-error positions.
 * ASCII space cannot be a UTF-8 continuation byte. Word splitting is therefore
 * exact even when the following word contains malformed UTF-8, which its own
 * scalar validation rejects. Each completed word advances the finite byte offset.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool unicode_name(const unsigned char *bytes, size_t length, size_t *error_offset) {
    if (bytes == NULL || length == 0) {
        *error_offset = 0;
        return false;
    }
    size_t offset = 0;
    for (;;) {
        size_t end = offset;
        while (end < length && bytes[end] != ' ') { end++; }
        size_t invalid = 0;
        if (!unicode_identifier(bytes + offset, end - offset, &invalid)) {
            *error_offset = offset + invalid;
            return false;
        }
        if (end == length) { return true; }
        if (end + 1 == length) {
            *error_offset = end;
            return false;
        }
        offset = end + 1;
    }
}
