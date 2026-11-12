/**
 * @file
 * @brief Preserve printable UTF-8 while escaping malformed and control bytes.
 *
 * [NAME-03](../../../spec/implementation/compiler.md#name-03)
 * Never lose original byte identity through transliteration or replacement glyphs.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

bool diagnostic_write_bytes(FILE *output, const unsigned char *bytes, size_t length) {
    size_t offset = 0;
    while (offset < length) {
        uint32_t scalar;
        size_t width;
        bool valid = utf8_decode(bytes + offset, length - offset, &scalar, &width);
        if (!valid || scalar < 32 || (scalar >= 127 && scalar <= 159)) {
            if (fprintf(output, "\\x%02X", (unsigned int)bytes[offset]) < 0) {
                return false;
            }
            offset++;
        } else {
            if (fwrite(bytes + offset, 1, width, output) != width) {
                return false;
            }
            offset += width;
        }
    }
    return true;
}

bool diagnostic_write_text(FILE *output, const char *text) {
    return diagnostic_write_bytes(output, (const unsigned char *)text, strlen(text));
}
