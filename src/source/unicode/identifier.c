/**
 * @file
 * @brief Recognize XID words using exact binary searches over pinned immutable ranges.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * Word membership is separate from complete-name validation in name.c; T022
 * selects the exact token span before applying those predicates.
 */
#include "unicode.h"

/** Inclusive pinned scalar range; tables are sorted and disjoint for exact binary search. Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
typedef struct {
    uint32_t first;
    uint32_t last;
} UnicodeRange;

#include "xid_tables.inc"

/** Binary-search sorted disjoint closed ranges. Shrinking interval preserves exact membership; no sampled or host-Unicode fallback.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
static bool contains(const UnicodeRange *ranges, size_t count, uint32_t scalar) {
    size_t lower = 0;
    size_t upper = count;
    while (lower < upper) {
        size_t middle = lower + (upper - lower) / 2;
        if (scalar < ranges[middle].first) {
            upper = middle;
        } else if (scalar > ranges[middle].last) {
            lower = middle + 1;
        } else {
            return true;
        }
    }
    return false;
}

bool unicode_identifier_start(uint32_t scalar) {
    return scalar == '_' || contains(xid_start,
        sizeof(xid_start) / sizeof(xid_start[0]), scalar);
}

bool unicode_identifier_continue(uint32_t scalar) {
    return scalar == '_' || contains(xid_continue,
        sizeof(xid_continue) / sizeof(xid_continue[0]), scalar);
}

bool unicode_identifier(const unsigned char *bytes, size_t length, size_t *error_offset) {
    size_t offset = 0;
    do {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes == NULL ? NULL : bytes + offset, length - offset, &scalar, &width)
            || !(offset == 0 ? unicode_identifier_start(scalar)
                : unicode_identifier_continue(scalar))) {
            *error_offset = offset;
            return false;
        }
        offset += width;
    } while (offset < length);
    return true;
}
