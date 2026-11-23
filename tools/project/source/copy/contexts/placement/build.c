/** @file
 * @brief Complete forced structural parent proposals over genuine immutable original facts.
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires one positional ancestor mapping; [ARCH-05](../../../../../../spec/implementation/compiler.md#arch-05)
 * requires an exact finite construction with retained failure evidence.
 */
#include "internal.h"
SourceCopyParentStatus source_copy_context_place(const SourceCopyParentInputs *inputs, SourceCopyParentIssue *issue) {
    if (!issue) { return SOURCE_COPY_PARENT_INVALID; } *issue = (SourceCopyParentIssue){0};
    if (!inputs || !inputs->copy || !inputs->copy->valid || !inputs->mains || inputs->mains->ready
        || inputs->mains->status != SOURCE_CLONE_OK || inputs->mains->capture != inputs->copy->tree->capture
        || !inputs->observed || !inputs->observed->valid || !inputs->observed->sealed
        || inputs->observed->original != inputs->mains->capture || (inputs->root_count && !inputs->roots)
        || (inputs->removal && (!inputs->removal->valid || inputs->removal->tree->capture != inputs->mains->capture
            || inputs->removal->analysis != inputs->copy->expansion.analysis))) { return SOURCE_COPY_PARENT_INVALID; }
    /* Every newly added context row has only a subset of the requester's final
     * parents, which that scan already visits. Sorted insertion before a visited
     * row therefore cannot hide a new obligation. Injectivity bounds additions
     * by original units; no iteration limit or discovery-order fallback is used. */
    for (const SourceCloneMapping *row = inputs->mains->first; row; row = row->next) {
        SourceCopyParentStatus status = source_copy_parent_scan(inputs, row, issue);
        if (status == SOURCE_COPY_PARENT_OK) { continue; }
        if (inputs->mains->status == SOURCE_CLONE_OK) {
            inputs->mains->status = status == SOURCE_COPY_PARENT_MEMORY ? SOURCE_CLONE_MEMORY : SOURCE_CLONE_INVALID;
        }
        return status;
    }
    *issue = (SourceCopyParentIssue){0}; return SOURCE_COPY_PARENT_OK;
}
