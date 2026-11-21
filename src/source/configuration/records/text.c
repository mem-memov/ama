/**
 * @file
 * @brief Compare complete INI spellings without temporary strings or locale rules.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires exact fixed
 * keys; [PKG-04](../../../../spec/project/packages.md#pkg-04) forbids approximate
 * alias matches or case folding. Bounded views preserve embedded data bytes.
 */
#include <string.h>
#include "internal.h"

bool configuration_text_is(SourceSpan text, const char *literal) {
    size_t length = strlen(literal);
    return source_span_valid(text) && text.end - text.start == length
        && memcmp(source_bytes(text.source) + text.start, literal, length) == 0;
}

bool configuration_text_equal(SourceSpan left, SourceSpan right) {
    return source_span_valid(left) && source_span_valid(right)
        && left.end - left.start == right.end - right.start
        && memcmp(source_bytes(left.source) + left.start,
            source_bytes(right.source) + right.start, left.end - left.start) == 0;
}
