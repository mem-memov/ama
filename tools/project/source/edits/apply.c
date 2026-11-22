/** @file
 * @brief Assemble one prepared source with checked sizes and exact unchanged byte segments.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves unrelated bytes; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates immutable prepared sources from filesystem publication.
 */
#include <stdint.h>
#include <string.h>
#include "edits.h"
SourceEditsStatus source_text_edits_apply(SourceTextEdits *edits, SourceManager *manager,
    const char *path, const SourceText **output) {
    if (!edits || edits->attempted) { return SOURCE_EDITS_INVALID; }
    if (edits->status != SOURCE_EDITS_OK) { return edits->status; }
    if (!edits->original || !manager || !path || !output) { return edits->status = SOURCE_EDITS_INVALID; }
    edits->attempted = true;
    size_t length = source_length(edits->original), total = length;
    for (const SourceTextEdit *row = edits->first; row; row = row->next) { total -= row->original.end - row->original.start; }
    for (const SourceTextEdit *row = edits->first; row; row = row->next) {
        if (row->length > SIZE_MAX - total) { return edits->status = SOURCE_EDITS_LIMIT; }
        total += row->length;
    }
    if (total == SIZE_MAX) { return edits->status = SOURCE_EDITS_LIMIT; }
    SourceArena scratch; source_arena_init(&scratch, edits->arena.allocator);
    unsigned char *bytes = source_arena_allocate(&scratch, total);
    if (!bytes) { source_arena_destroy(&scratch); return edits->status = SOURCE_EDITS_MEMORY; }
    size_t input = 0, used = 0;
    const unsigned char *original = source_bytes(edits->original);
    for (const SourceTextEdit *row = edits->first; row; row = row->next) {
        size_t retained = row->original.start - input;
        memcpy(bytes + used, original + input, retained); used += retained;
        if (row->length) { memcpy(bytes + used, row->bytes, row->length); used += row->length; }
        input = row->original.end;
    }
    memcpy(bytes + used, original + input, length - input);
    const SourceText *result = NULL;
    SourceStatus status = source_manager_add(manager, path, bytes, total, &result);
    source_arena_destroy(&scratch);
    if (status != SOURCE_OK) { return edits->status = status == SOURCE_LIMIT ? SOURCE_EDITS_LIMIT : SOURCE_EDITS_MEMORY; }
    edits->result = result; edits->valid = true; *output = result;
    return SOURCE_EDITS_OK;
}
