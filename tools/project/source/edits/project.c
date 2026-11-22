/** @file
 * @brief Translate occurrence boundaries through admitted edits without guessing replaced interiors.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact source sites after operand edits. Unsigned remove-then-add
 * arithmetic avoids signed offset deltas for shrinking and expanding paths.
 */
#include <stdint.h>
#include "edits.h"
/** Completed ranges remove old bytes; inserted bytes use explicit boundary bias. */
static bool point(const SourceTextEdits *edits, size_t position, bool right, size_t *output) {
    size_t result = position;
    for (const SourceTextEdit *row = edits->first; row; row = row->next) {
        SourceSpan span = row->original;
        if (span.start < position && position < span.end) { return false; }
        if (span.end <= position) { result -= span.end - span.start; }
    }
    for (const SourceTextEdit *row = edits->first; row; row = row->next) {
        SourceSpan span = row->original;
        bool insertion = span.start == span.end;
        bool included = insertion ? span.start < position || (right && span.start == position) : span.end <= position;
        if (included) {
            if (row->length > SIZE_MAX - result) { return false; }
            result += row->length;
        }
    }
    *output = result; return true;
}
bool source_text_edits_project(const SourceTextEdits *edits, SourceSpan original, SourceSpan *output) {
    if (!edits || !edits->valid || original.source != edits->original || !source_span_valid(original) || !output) { return false; }
    SourceSpan result = {.source = edits->result};
    if (!point(edits, original.start, true, &result.start)
        || !point(edits, original.end, original.start == original.end, &result.end) || !source_span_valid(result)) { return false; }
    *output = result; return true;
}
