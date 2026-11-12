/**
 * @file
 * @brief Decode shortest-form Unicode scalars and report exact malformed byte offsets.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * Reject surrogates, overlong/truncated forms and out-of-range scalars without locale rules.
 */
#include "unicode.h"

/** Partition legal UTF-8 lead bytes by width; zero rejects continuation, overlong and out-of-range lead bytes.
 */
static size_t sequence_width(unsigned char first) {
    if (first < 0x80) {
        return 1;
    }
    if (first >= 0xc2 && first <= 0xdf) {
        return 2;
    }
    if (first >= 0xe0 && first <= 0xef) {
        return 3;
    }
    if (first >= 0xf0 && first <= 0xf4) {
        return 4;
    }
    return 0;
}

/** Fold continuation groups and check minimal value/scalar domain before publishing either decoded output.
 */
bool utf8_decode(const unsigned char *bytes, size_t length,
    uint32_t *scalar, size_t *width) {
    if (length == 0 || bytes == NULL) {
        return false;
    }
    size_t count = sequence_width(bytes[0]);
    if (count == 0 || count > length) {
        return false;
    }
    uint32_t value = count == 1 ? bytes[0] : bytes[0] & (0x7fU >> count);
    for (size_t index = 1; index < count; index++) {
        if ((bytes[index] & 0xc0) != 0x80) {
            return false;
        }
        value = (value << 6) | (bytes[index] & 0x3f);
    }
    const uint32_t minimum[] = {0, 0, 0x80, 0x800, 0x10000};
    if (value < minimum[count] || value > 0x10ffff
        || (value >= 0xd800 && value <= 0xdfff)) {
        return false;
    }
    *scalar = value;
    *width = count;
    return true;
}

bool utf8_validate(const unsigned char *bytes, size_t length, size_t *error_offset) {
    size_t offset = 0;
    while (offset < length) {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes == NULL ? NULL : bytes + offset,
                length - offset, &scalar, &width)) {
            *error_offset = offset;
            return false;
        }
        offset += width;
    }
    return true;
}
