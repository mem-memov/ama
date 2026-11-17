/**
 * @file
 * @brief Scan one bounded Ama word with exact pinned scalar and internal-hyphen rules.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) adds U+002D
 * between ordinary word characters without changing Unicode property tables.
 * Consume a hyphen only together with its required continuation scalar, so the
 * published prefix always ends in an ordinary character and never needs repair.
 * [TPL-01](../../../spec/language/templates.md#tpl-01) reuses these same words
 * inside descriptive static-name components; dots and roots stay with callers.
 */
#include "unicode.h"

/** Scan the longest word prefix, keeping every published offset after a complete
 * ordinary scalar. A hyphen transition consumes its following continuation in
 * the same step, so two adjoining hyphens and an unfinished hyphen cannot enter
 * the result. Each successful step advances a bounded positive byte count;
 * UTF-8/property failure leaves the preceding complete prefix unchanged.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * requires this exact acceptance for both complete validation and token spans.
 */
size_t unicode_word_prefix(const unsigned char *bytes, size_t length) {
    uint32_t scalar;
    size_t width;
    if (!utf8_decode(bytes, length, &scalar, &width)
        || !unicode_identifier_start(scalar)) {
        return 0;
    }
    size_t offset = width;
    while (offset < length) {
        size_t continuation = offset;
        if (bytes[offset] == '-') {
            continuation++;
        }
        if (!utf8_decode(bytes + continuation, length - continuation, &scalar, &width)
            || !unicode_identifier_continue(scalar)) {
            break;
        }
        offset = continuation + width;
    }
    return offset;
}

/** Validate the whole word using the same prefix kernel as the lexer, preserving
 * the error output on success and reporting its first unconsumed byte on failure.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * requires exact full-span recognition without trimming an invalid suffix.
 */
bool unicode_identifier(const unsigned char *bytes, size_t length, size_t *error_offset) {
    size_t prefix = unicode_word_prefix(bytes, length);
    if (prefix != 0 && prefix == length) {
        return true;
    }
    *error_offset = prefix;
    return false;
}
