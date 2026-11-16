/**
 * @file
 * @brief Read native C identifier scalars/UCNs and reject exact C keyword spellings.
 *
 * [PROTO-11](../../../../spec/implementation/native-interface.md#proto-11)
 * Native identifiers use the supported C lexical domain, independently of Ama XID names.
 */
#include <stdint.h>
#include "../internal.h"
#include "source/unicode/unicode.h"

/** Decode exactly one ASCII hexadecimal digit, returning -1 for every other byte.
 * Component contract: [PROTO-11](../../../../spec/implementation/native-interface.md#proto-11). */
static int hex_digit(unsigned char byte) {
    return byte >= '0' && byte <= '9' ? byte - '0'
        : byte >= 'a' && byte <= 'f' ? byte - 'a' + 10
        : byte >= 'A' && byte <= 'F' ? byte - 'A' + 10 : -1;
}

/** Decode one UTF-8 scalar or fixed-width C UCN. Validate digit count and restricted low values before publishing scalar/width.
 * Component contract: [PROTO-11](../../../../spec/implementation/native-interface.md#proto-11). */
static bool read_scalar(const unsigned char *bytes, size_t length, uint32_t *scalar, size_t *width) {
    if (length == 0 || bytes[0] != '\\') {
        return utf8_decode(bytes, length, scalar, width);
    }
    size_t digits = length > 1 && bytes[1] == 'u' ? 4 : length > 1 && bytes[1] == 'U' ? 8 : 0;
    if (digits == 0 || length < digits + 2) {
        return false;
    }
    uint32_t value = 0;
    for (size_t index = 0; index < digits; index++) {
        int digit = hex_digit(bytes[index + 2]);
        if (digit < 0) {
            return false;
        }
        value = (value << 4) | (uint32_t)digit;
    }
    if (value < 0xa0 && value != '$') {
        return false;
    }
    *scalar = value;
    *width = digits + 2;
    return true;
}

/** Reject exact supported C11 keyword spellings using a fixed finite set; no locale or naming-prefix heuristic.
 * Component contract: [PROTO-11](../../../../spec/implementation/native-interface.md#proto-11). */
static bool keyword(SourceSpan text) {
    static const char *const words[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
        "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long",
        "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct",
        "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas",
        "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn",
        "_Static_assert", "_Thread_local"
    };
    for (size_t index = 0; index < sizeof(words) / sizeof(words[0]); index++) {
        if (layout_text_is(text, words[index])) {
            return true;
        }
    }
    return false;
}

bool abi_identifier(SourceSpan text, size_t *error_offset) {
    size_t cursor = text.start;
    const unsigned char *bytes = source_bytes(text.source);
    while (cursor < text.end) {
        uint32_t scalar;
        size_t width;
        if (!read_scalar(bytes + cursor, text.end - cursor, &scalar, &width)
            || !abi_identifier_scalar(scalar, cursor == text.start)) {
            *error_offset = cursor;
            return false;
        }
        cursor += width;
    }
    if (cursor == text.start || keyword(text)) {
        *error_offset = text.start;
        return false;
    }
    return true;
}
