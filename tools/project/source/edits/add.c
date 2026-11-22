/** @file
 * @brief Admit exact nonoverlapping original ranges without token or name searches.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits only prepared parsed-span edits; equal repeats preserve one operation,
 * while contradictory overlapping operations refuse before byte publication.
 */
#include <stdint.h>
#include <string.h>
#include "edits.h"
/** Half-open overlap additionally rejects insertion strictly inside a replacement. */
static bool overlap(SourceSpan left, SourceSpan right) {
    if (left.start == left.end) { return right.start < left.start && left.start < right.end; }
    if (right.start == right.end) { return left.start < right.start && right.start < left.end; }
    return left.start < right.end && right.start < left.end;
}
SourceEditsStatus source_text_edits_add(SourceTextEdits *edits, SourceSpan span,
    const unsigned char *bytes, size_t length) {
    if (!edits || edits->attempted) { return SOURCE_EDITS_INVALID; }
    if (edits->status != SOURCE_EDITS_OK) { return edits->status; }
    edits->failed = span;
    if (!edits->original || span.source != edits->original || !source_span_valid(span) || (length && !bytes)) {
        return edits->status = SOURCE_EDITS_INVALID;
    }
    if (length == SIZE_MAX) { return edits->status = SOURCE_EDITS_LIMIT; }
    for (const SourceTextEdit *row = edits->first; row; row = row->next) {
        bool exact = span.start == row->original.start && span.end == row->original.end;
        if (exact && length == row->length && (!length || !memcmp(bytes, row->bytes, length))) { return SOURCE_EDITS_OK; }
        if (exact || overlap(span, row->original)) {
            edits->conflict = row; return edits->status = SOURCE_EDITS_CONFLICT;
        }
    }
    SourceTextEdit *row = source_arena_allocate(&edits->arena, sizeof(*row));
    unsigned char *copy = row && length ? source_arena_allocate(&edits->arena, length) : NULL;
    if (!row || (length && !copy)) { return edits->status = SOURCE_EDITS_MEMORY; }
    if (length) { memcpy(copy, bytes, length); }
    SourceTextEdit **position = &edits->first;
    while (*position && ((*position)->original.start < span.start
        || ((*position)->original.start == span.start && (*position)->original.end < span.end))) { position = &(*position)->next; }
    *row = (SourceTextEdit){.next = *position, .original = span, .bytes = copy, .length = length};
    *position = row;
    return SOURCE_EDITS_OK;
}
