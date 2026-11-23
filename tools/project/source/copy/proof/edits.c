/** @file
 * @brief Restrict copied main-file edits to their authoritative written module operands.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves literals, comments, labels and storage suffixes. This interval gate
 * composes with exact projected final sites, rather than treating a successful
 * compiler response as permission to alter other source bytes.
 */
#include "internal.h"
#include <string.h>
/** A nonempty edit may touch operand boundaries; an insertion must be strictly
 * inside because site projection excludes insertions at outer boundaries. */
static bool inside(SourceSpan edit, SourceSpan operand) {
    return edit.source == operand.source && edit.start >= operand.start && edit.end <= operand.end
        && (edit.start != edit.end || (edit.start > operand.start && edit.start < operand.end));
}
/** Only a written module operand grants edit authority; requiring constructs
 * and their literal/variable regions remain unchanged even if targets agree. */
static bool allowed(const DeliverySourceDefinition *definition, SourceSpan edit) {
    for (const DeliverySourceDependency *edge = definition ? definition->dependencies : NULL; edge; edge = edge->next) {
        if (!edge->implicit && inside(edit, edge->origin)) { return true; }
    }
    return false;
}
bool source_copy_proof_edits(const DeliverySourceDefinition *original, const SourceCloneMap *map,
    const SourceGraphPreparedText *prepared, SourceCopyProofIssue *issue) {
    for (const SourceGraphPreparedText *row = prepared; row; row = row->next) {
        const char *path = source_path(row->edits->result);
        const SourceCloneMapping *copy = map->first;
        while (copy && strcmp(copy->path, path)) { copy = copy->next; }
        if (!copy) { continue; }
        const DeliverySourceDefinition *definition = original;
        while (definition && strcmp(definition->identity, copy->original->logical_path)) { definition = definition->next; }
        for (const SourceTextEdit *edit = row->edits->first; edit; edit = edit->next) {
            if (!allowed(definition, edit->original)) {
                issue->path = path; issue->prepared = row; issue->edit = edit; return false;
            }
        }
    }
    return true;
}
