/**
 * @file
 * @brief Validate absolute project/standard module paths with exact multiword Unicode components.
 *
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) distinguishes
 * single/double-dot namespace selectors; [SRC-03](../../../../spec/language/source-and-names.md#src-03)
 * applies complete-name rules to each dot-delimited component without trimming.
 */
#include "../catalog.h"
#include "source/unicode/unicode.h"

/** Consume exactly one root marker, then every nonempty complete component.
 * Every accepted separator advances a byte index; malformed or empty components
 * fail before another iteration. Physical filesystem traversal is never involved.
 * Component contract: [SRC-01](../../../../spec/language/source-and-names.md#src-01), [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
bool module_path_valid(const char *path) {
    if (path == NULL || path[0] != '.') { return false; }
    size_t start = path[1] == '.' ? 2 : 1;
    for (;;) {
        size_t end = start;
        while (path[end] != 0 && path[end] != '.') { end++; }
        size_t error;
        if (!unicode_name((const unsigned char *)path + start, end - start, &error)) { return false; }
        if (path[end] == 0) { return true; }
        start = end + 1;
    }
}
