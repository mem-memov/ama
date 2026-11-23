/** @file
 * @brief Reconstruct complete C with the same positional parent operation used by commands.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * forbids ambiguous parent origins; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact clone correspondence, not merely well-typed final bytes.
 */
#include "internal.h"
#include <string.h>
/** Preserve actual allocation-versus-correspondence failure on seed insertion. */
static SourceOperationMaterialStatus add(SourceOperationMaterialReview *review, const ModuleSource *original,
    const char *path, const char *identity) {
    const SourceCloneMapping *added = NULL;
    SourceCloneStatus status = source_clone_map_add(&review->expected_mains, original, path, identity, &added);
    return status == SOURCE_CLONE_OK ? SOURCE_OPERATION_MATERIAL_OK
        : status == SOURCE_CLONE_MEMORY ? SOURCE_OPERATION_MATERIAL_MEMORY : SOURCE_OPERATION_MATERIAL_CONTEXT;
}
/** Actual final capture determines the main's physical key after independent
 * common discovery. No second image, synthetic topology or guessed stem occurs. */
static SourceOperationMaterialStatus active(SourceOperationMaterialReview *review) {
    for (size_t i = 0; i < review->destinations.count; i++) {
        const SourceCopyDestination *row = &review->destinations.entries[i];
        const SourceCapturedUnit *unit = review->plan->capture.units;
        while (unit && strcmp(unit->source->logical_path, row->identity)) { unit = unit->next; }
        review->failed_main = row->source; review->failed_path = row->identity;
        if (!unit) { return SOURCE_OPERATION_MATERIAL_CONTEXT; }
        SourceOperationMaterialStatus status = add(review, row->source, source_path(unit->source->text), row->identity);
        if (status != SOURCE_OPERATION_MATERIAL_OK) { return status; }
    }
    return SOURCE_OPERATION_MATERIAL_OK;
}
/** Compare both finite relation inclusions using genuine original handles. */
static bool included(const SourceCloneMap *left, const SourceCloneMap *right) {
    for (const SourceCloneMapping *row = left->first; row; row = row->next) {
        const SourceCloneMapping *match = right->first;
        while (match && match->original != row->original) { match = match->next; }
        if (!match || strcmp(match->path, row->path) || strcmp(match->identity, row->identity)) { return false; }
    }
    return true;
}
SourceOperationMaterialStatus source_operation_material_contexts(SourceOperationMaterialReview *review) {
    if (!review->plan->inputs.copy) { return SOURCE_OPERATION_MATERIAL_OK; }
    for (const SourceCloneMapping *row = review->explicit_mains.first; row; row = row->next) {
        SourceOperationMaterialStatus status = add(review, row->original, row->path, row->identity);
        if (status != SOURCE_OPERATION_MATERIAL_OK) { return status; }
    }
    SourceOperationMaterialStatus status = active(review);
    if (status != SOURCE_OPERATION_MATERIAL_OK) { return status; }
    SourceCopyParentInputs parents = {.copy = review->plan->inputs.copy, .mains = &review->expected_mains,
        .observed = review->plan->inputs.observed, .removal = review->plan->inputs.removal,
        .roots = review->plan->inputs.final_roots, .root_count = review->plan->inputs.final_root_count};
    review->parent_status = source_copy_context_place(&parents, &review->parent_issue);
    if (review->parent_status != SOURCE_COPY_PARENT_OK) {
        review->failed_path = review->parent_issue.path; review->failed_main = review->parent_issue.required;
        return review->parent_status == SOURCE_COPY_PARENT_MEMORY ? SOURCE_OPERATION_MATERIAL_MEMORY : SOURCE_OPERATION_MATERIAL_CONTEXT;
    }
    if (source_clone_map_seal(&review->expected_mains) != SOURCE_CLONE_OK
        || !included(&review->expected_mains, review->plan->inputs.mains)
        || !included(review->plan->inputs.mains, &review->expected_mains)) { return SOURCE_OPERATION_MATERIAL_CONTEXT; }
    review->failed_main = NULL; review->failed_path = NULL; return SOURCE_OPERATION_MATERIAL_OK;
}
