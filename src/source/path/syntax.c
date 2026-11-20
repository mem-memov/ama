/**
 * @file
 * @brief Scan structured raw path prefixes without declaration-dependent splits.
 * [SRC-06](../../../spec/language/source-and-names.md#src-06) requires recognizing
 * path roots before normalization; [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * preserves complete multiword components. Catalog identity validation is separate.
 */
#include "syntax.h"
#include "version.h"
#include "source/unicode/unicode.h"

/** Select a component by its disjoint leading syntax; v-prefixed names can also
 * be versions, so take the longer grammatical prefix. Both scans are linear
 * over that component, keeping total work linear without speculative lookup.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) supplies version punctuation;
 * [SRC-06](../../../spec/language/source-and-names.md#src-06) allows whole dot and parent components.
 */
static size_t component(const unsigned char *bytes, size_t length) {
    if (length == 0) { return 0; }
    if (bytes[0] == '.') {
        return length > 1 && bytes[1] == '.' ? 2 : 1;
    }
    size_t name = unicode_name_prefix(bytes, length);
    size_t version = module_version_prefix(bytes, length);
    return name > version ? name : version;
}

/** Every component consumes positive bytes and every continuation consumes one
 * slash. No recursion, candidate storage or numeric conversion is needed. The
 * cursor never exceeds length, and no incomplete slash suffix is published.
 * [SRC-06](../../../spec/language/source-and-names.md#src-06) leaves normalization
 * to its owner rather than guessing a filesystem interpretation here.
 */
size_t module_path_prefix(const unsigned char *bytes, size_t length) {
    if (bytes == NULL || length == 0) { return 0; }
    size_t cursor;
    if (bytes[0] == '~') {
        size_t name = unicode_name_prefix(bytes + 1, length - 1);
        if (name == 0) { return 0; }
        cursor = 1 + name;
        if (cursor == length || bytes[cursor] != '/') { return cursor; }
        cursor++;
    } else if (bytes[0] == '/') {
        cursor = length > 1 && bytes[1] == '/' ? 2 : 1;
    } else if (length >= 2 && bytes[0] == '.' && bytes[1] == '/') {
        cursor = 2;
    } else if (length >= 3 && bytes[0] == '.' && bytes[1] == '.' && bytes[2] == '/') {
        cursor = 3;
    } else {
        return 0;
    }
    for (;;) {
        size_t width = component(bytes + cursor, length - cursor);
        if (width == 0) { return 0; }
        cursor += width;
        if (cursor == length || bytes[cursor] != '/') { return cursor; }
        cursor++;
    }
}

/** Whole-span callers reject every unconsumed byte, including embedded NUL and
 * malformed Unicode, as required by [SRC-03](../../../spec/language/source-and-names.md#src-03).
 */
bool module_path_syntax(const unsigned char *bytes, size_t length) {
    size_t width = module_path_prefix(bytes, length);
    return width != 0 && width == length;
}
