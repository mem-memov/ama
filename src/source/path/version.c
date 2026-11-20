/**
 * @file
 * @brief Validate a complete version's decimal fields and optional Unicode variant.
 *
 * [PKG-01](../../../spec/project/packages.md#pkg-01) requires exactly three
 * canonical nonnegative decimal fields. Reuse
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)'s pinned word
 * predicate only for the variant, which cannot contain spaces or leading digits.
 */
#include "version.h"
#include "source/unicode/unicode.h"

/** Consume a nonempty canonical decimal field from a bounded cursor.
 * Every increment is guarded by cursor < length; no arithmetic representation
 * of the number is constructed. A leading zero may consume only one digit.
 * The caller owns the cursor and discards it on false.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) supplies these exact lexical
 * conditions independently of numeric size or the following punctuation.
 */
static bool decimal_field(const unsigned char *bytes, size_t length, size_t *cursor) {
    size_t start = *cursor;
    while (*cursor < length && bytes[*cursor] >= '0' && bytes[*cursor] <= '9') {
        (*cursor)++;
    }
    return *cursor != start && (bytes[start] != '0' || *cursor - start == 1);
}

/** Validate in one forward pass; bounded decimal scans and the final word scan
 * cover disjoint spans. Each successful field/separator advances the cursor,
 * establishing termination without integer conversion or trial alternatives.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) fixes the three-field shape;
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) validates all variant
 * scalars, including isolated internal hyphens, without normalizing their bytes.
 */
bool module_version_component(const unsigned char *bytes, size_t length) {
    if (bytes == NULL || length == 0 || bytes[0] != 'v') {
        return false;
    }
    size_t cursor = 1;
    for (size_t field = 0; field < 3; field++) {
        if (!decimal_field(bytes, length, &cursor)) {
            return false;
        }
        if (field != 2) {
            if (cursor == length || bytes[cursor] != '.') {
                return false;
            }
            cursor++;
        }
    }
    if (cursor == length) {
        return true;
    }
    if (bytes[cursor] != '-') {
        return false;
    }
    cursor++;
    size_t error_offset;
    return unicode_identifier(bytes + cursor, length - cursor, &error_offset);
}
