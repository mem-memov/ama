/** @file
 * @brief Preserve exact applied text producer identity in one owned immutable query list.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * authorizes main operands, copied aliases and carried tables through distinct
 * known producers, not arbitrary applied companion text.
 */
#include "internal.h"
#include <string.h>
const SourceFileMapping *source_operation_writes_file(const SourceFileMap *files, const char *path) {
    for (const SourceFileMapping *row = files->first; row; row = row->next) {
        if (!strcmp(row->destination, path)) { return row; }
    }
    return NULL;
}
SourceOperationWritesStatus source_operation_writes_text(SourceOperationWrites *writes, const SourceOperationPreparedWrite *proposed) {
    if (!proposed || proposed->next || proposed->text.next || proposed->kind < SOURCE_OPERATION_PREPARED_MAIN
        || proposed->kind > SOURCE_OPERATION_PREPARED_CONFIGURATION) { return SOURCE_OPERATION_WRITES_INVALID; }
    const SourceTextEdits *edits = proposed->text.edits;
    if (!edits || !edits->valid || !edits->result) { return SOURCE_OPERATION_WRITES_INVALID; }
    if (proposed->kind == SOURCE_OPERATION_PREPARED_MAIN) {
        if (!proposed->owner.main || proposed->owner.main->edits != edits) { return SOURCE_OPERATION_WRITES_INVALID; }
    } else if (proposed->kind == SOURCE_OPERATION_PREPARED_MODULE) {
        if (!proposed->owner.module || &proposed->owner.module->edits != edits) { return SOURCE_OPERATION_WRITES_INVALID; }
    } else if (!proposed->owner.configuration || &proposed->owner.configuration->edits != edits) { return SOURCE_OPERATION_WRITES_INVALID; }
    const char *path = source_path(edits->result); writes->failed_path = path;
    const SourceFileMapping *file = source_operation_writes_file(writes->inputs.files, path);
    if (!file || file->original->directory) { return SOURCE_OPERATION_WRITES_SCOPE; }
    for (const SourceOperationPreparedWrite *row = writes->first; row; row = row->next) {
        if (!strcmp(source_path(row->text.edits->result), path)) { return SOURCE_OPERATION_WRITES_DUPLICATE; }
    }
    if (proposed->kind == SOURCE_OPERATION_PREPARED_MAIN) {
        const SourceCloneMapping *clone = writes->inputs.mains->first;
        while (clone && strcmp(clone->path, path)) { clone = clone->next; }
        if (!clone) { return SOURCE_OPERATION_WRITES_SCOPE; }
    }
    SourceOperationPreparedWrite *row = source_arena_allocate(&writes->arena, sizeof(*row));
    if (!row) { return SOURCE_OPERATION_WRITES_MEMORY; }
    *row = *proposed;
    if (writes->last) { writes->last->next = row; writes->last->text.next = &row->text; }
    else { writes->first = row; writes->prepared = &row->text; }
    writes->last = row; return SOURCE_OPERATION_WRITES_OK;
}
