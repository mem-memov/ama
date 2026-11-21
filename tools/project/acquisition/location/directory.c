/** @file
 * @brief Reduce physical relative directory components with a terminating bounded stack.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) forbids checkout escape;
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) requires
 * representable allocation and exact finite processing, not guessed path caps.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
AcquisitionMapStatus acquisition_map_failure(SourceIssue *issue, SourceSpan span,
    AcquisitionMapStatus status, const char *message) {
    *issue = (SourceIssue){source_span_valid(span) ? span : (SourceSpan){0}, message,
        "a contained relative repository directory and representable owned storage"};
    return status;
}
AcquisitionMapStatus acquisition_directory(SourceArena *arena, SourceSpan expression,
    const char **output, size_t *output_length, SourceIssue *issue) {
    if (!source_span_valid(expression) || expression.start == expression.end) {
        return acquisition_map_failure(issue, expression, ACQUISITION_MAP_INVALID, "Missing repository directory metadata");
    }
    const unsigned char *bytes = source_bytes(expression.source) + expression.start;
    size_t length = expression.end - expression.start;
    if (bytes[0] == '/' || memchr(bytes, 0, length)) {
        return acquisition_map_failure(issue, expression, ACQUISITION_MAP_INVALID, "Repository directory is absolute or contains a zero byte");
    }
    if (length == SIZE_MAX) {
        return acquisition_map_failure(issue, expression, ACQUISITION_MAP_LIMIT, "Repository directory storage is not representable");
    }
    char *normalized = source_arena_allocate(arena, length + 1);
    if (!normalized) {
        return acquisition_map_failure(issue, expression, ACQUISITION_MAP_OUT_OF_MEMORY, "Cannot allocate repository directory normalization");
    }
    size_t used = 0;
    for (size_t cursor = 0; cursor < length;) {
        size_t end = cursor;
        while (end < length && bytes[end] != '/') { end++; }
        size_t width = end - cursor;
        if (width == 2 && bytes[cursor] == '.' && bytes[cursor + 1] == '.') {
            if (!used) { return acquisition_map_failure(issue, expression, ACQUISITION_MAP_INVALID, "Repository directory escapes the checkout"); }
            /* Each pushed byte can be removed at most once, so backward pops
             * preserve linear amortized work across arbitrarily many parents. */
            while (used && normalized[used - 1] != '/') { used--; }
            if (used) { used--; }
        } else if (width && !(width == 1 && bytes[cursor] == '.')) {
            if (used) { normalized[used++] = '/'; }
            memcpy(normalized + used, bytes + cursor, width); used += width;
        }
        if (end == length) { break; }
        cursor = end + 1;
    }
    normalized[used] = 0;
    *output = normalized; *output_length = used;
    return ACQUISITION_MAP_OK;
}
