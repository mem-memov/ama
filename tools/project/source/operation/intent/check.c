/** @file
 * @brief Admit genuine source/copy/deletion owners before reviewing exact operation intent.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes one admitted original analysis domain for the entire operation.
 */
#include "internal.h"
SourceOperationIntentStatus source_operation_intent_check(const SourceOperationPlan *plan,
    const SourceOperationIntent *intent, SourceOperationIntentIssue *issue) {
    if (!issue) { return SOURCE_OPERATION_INTENT_INVALID; }
    *issue = (SourceOperationIntentIssue){0};
    if (!plan || !plan->prepared || !intent || !intent->source || !intent->source->valid
        || intent->source->capture != plan->inputs.observed->original
        || intent->kind < SOURCE_OPERATION_COPY || intent->kind > SOURCE_OPERATION_MOVE
        || (intent->replacement_count && !intent->replacements)) { return SOURCE_OPERATION_INTENT_INVALID; }
    if (source_tree_selection_domain(intent->source, &issue->path) != SOURCE_TREE_DOMAIN_OK) {
        return SOURCE_OPERATION_INTENT_DOMAIN;
    }
    if (intent->kind == SOURCE_OPERATION_DELETE) {
        if (plan->inputs.copy || intent->placement || intent->replace || intent->replacement_count) {
            return SOURCE_OPERATION_INTENT_INVALID;
        }
    } else if (!plan->inputs.copy || plan->inputs.copy->tree != intent->source || !intent->placement
        || !intent->placement->ready || intent->placement->selection != intent->source) {
        return SOURCE_OPERATION_INTENT_INVALID;
    }
    SourceOperationIntentStatus status = intent->kind == SOURCE_OPERATION_DELETE ? SOURCE_OPERATION_INTENT_OK
        : source_operation_intent_placement(plan, intent, issue);
    if (status == SOURCE_OPERATION_INTENT_OK) { status = source_operation_intent_removal(plan, intent, issue); }
    if (status == SOURCE_OPERATION_INTENT_OK) { *issue = (SourceOperationIntentIssue){0}; }
    return status;
}
