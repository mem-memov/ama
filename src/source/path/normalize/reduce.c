/**
 * @file
 * @brief Apply exact dot/parent/push reductions to an admitted component stream.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) refuses root
 * escape and never requires a cancelled intermediate component to exist.
 */
#include "internal.h"

ModulePathStatus path_normalization_reduce(ModulePathFragment fragment, ModulePathPart *parts,
    size_t *count, SourceIssue *issue) {
    size_t cursor = 0;
    while (cursor < fragment.length) {
        size_t end = cursor;
        while (end < fragment.length && fragment.bytes[end] != '/') { end++; }
        size_t length = end - cursor;
        SourceSpan origin = fragment.origin.source == NULL ? (SourceSpan){0}
            : (SourceSpan){fragment.origin.source, fragment.origin.start + cursor, fragment.origin.start + end};
        bool dot = length == 1 && fragment.bytes[cursor] == '.';
        bool parent = length == 2 && fragment.bytes[cursor] == '.' && fragment.bytes[cursor + 1] == '.';
        if (parent) {
            if (*count == 0) {
                return path_normalization_failure(issue, origin, MODULE_PATH_UNDERFLOW,
                    "Path crosses its namespace root", "a parent component with an existing parent");
            }
            (*count)--;
        } else if (!dot) {
            parts[*count] = (ModulePathPart){fragment.bytes + cursor, length, origin};
            (*count)++;
        }
        if (end == fragment.length) { break; }
        cursor = end + 1;
    }
    return MODULE_PATH_OK;
}
