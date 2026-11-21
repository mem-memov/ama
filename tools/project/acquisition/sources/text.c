/** @file
 * @brief Compare/copy immutable source-key bytes without URL/revision guesses.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) distinguishes exact
 * remote strings and absent revisions; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks every copied terminator before allocating.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
bool acquisition_source_text_equal(const char *text, SourceSpan span) {
    if (!span.source) { return text == NULL; }
    return text && source_span_valid(span) && strlen(text) == span.end - span.start &&
        !memcmp(text, source_bytes(span.source) + span.start, span.end - span.start);
}
char *acquisition_source_text(AcquisitionSources *sources, SourceSpan span) {
    if (!source_span_valid(span)) { return NULL; }
    size_t length = span.end - span.start;
    if (length == SIZE_MAX) { return NULL; }
    char *copy = source_arena_allocate(&sources->arena, length + 1);
    if (!copy) { return NULL; }
    memcpy(copy, source_bytes(span.source) + span.start, length); copy[length] = 0;
    return copy;
}
