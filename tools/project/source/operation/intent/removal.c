/** @file
 * @brief Distinguish selected source deletion from explicitly permitted destination replacement.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * keeps dependency originals when moving selected code;
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * requires the original joint cut even with replacement permission.
 */
#include "internal.h"
#include <string.h>
/** A genuine removal owner belongs to this exact original capture and analysis. */
static bool owner(const SourceOperationPlan *plan, const SourceRemovalSelection *cut) {
    return cut && cut->valid && cut->tree->capture == plan->inputs.observed->original
        && cut->analysis == plan->inputs.original;
}
/** Replacement begins at an explicit mapped destination; an arbitrary ancestor
 * does not acquire removal authority merely by containing a proposed write. */
static bool destination(const SourceOperationPlan *plan, const SourceOperationIntent *intent, const char *path) {
    if (!strcmp(path, intent->placement->destination)) { return true; }
    for (const SourceFileMapping *file = plan->inputs.files->first; file; file = file->next) {
        if (!strcmp(file->destination, path)) { return true; }
    }
    return false;
}
SourceOperationIntentStatus source_operation_intent_removal(const SourceOperationPlan *plan,
    const SourceOperationIntent *intent, SourceOperationIntentIssue *issue) {
    if (intent->kind == SOURCE_OPERATION_COPY) {
        if (intent->source_removal) { return SOURCE_OPERATION_INTENT_INVALID; }
    } else if (!owner(plan, intent->source_removal) || intent->source_removal->tree != intent->source) {
        return SOURCE_OPERATION_INTENT_INVALID;
    }
    if (intent->replacement_count && !intent->replace) { return SOURCE_OPERATION_INTENT_REPLACE; }
    for (size_t i = 0; i < intent->replacement_count; i++) {
        const SourceRemovalSelection *cut = intent->replacements[i]; issue->selection = cut;
        if (!owner(plan, cut)) { return SOURCE_OPERATION_INTENT_INVALID; }
        issue->path = cut->tree->operand.path;
        if (!destination(plan, intent, issue->path)) { return SOURCE_OPERATION_INTENT_REPLACE; }
    }
    return source_operation_intent_union(plan, intent, issue);
}
