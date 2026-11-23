/** @file
 * @brief Compare explicit raw-tree placement with the prepared physical relation and reject overlap.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes P from the actual selected tree, not textual logical-name replacement;
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * rejects source/destination self-overlap before publication.
 */
#include "internal.h"
#include <string.h>
/** Captured bytes and kind establish one raw input, not basename or pointer coincidence. */
static bool same(const AcquisitionUnitEntry *left, const AcquisitionUnitEntry *right) {
    return left->directory == right->directory && !strcmp(left->file.path, right->file.path)
        && (left->directory || (source_length(left->file.content) == source_length(right->file.content)
            && !memcmp(source_bytes(left->file.content), source_bytes(right->file.content), source_length(left->file.content))));
}
SourceOperationIntentStatus source_operation_intent_placement(const SourceOperationPlan *plan,
    const SourceOperationIntent *intent, SourceOperationIntentIssue *issue) {
    for (const SourceTreePlacementEntry *entry = intent->placement->entries; entry; entry = entry->next) {
        const SourceFileMapping *mapped = plan->inputs.files->first;
        while (mapped && strcmp(mapped->source, entry->original->file.path)) { mapped = mapped->next; }
        issue->path = entry->path;
        if (!mapped || strcmp(mapped->destination, entry->path) || !same(mapped->original, entry->original)) {
            return SOURCE_OPERATION_INTENT_PLACEMENT;
        }
    }
    for (const SourceFileMapping *mapped = plan->inputs.files->first; mapped; mapped = mapped->next) {
        for (const AcquisitionUnitEntry *raw = intent->source->members; raw; raw = raw->next) {
            if (source_image_below(mapped->destination, raw->file.path)
                || source_image_below(raw->file.path, mapped->destination)) {
                issue->path = mapped->destination; return SOURCE_OPERATION_INTENT_OVERLAP;
            }
        }
    }
    return SOURCE_OPERATION_INTENT_OK;
}
