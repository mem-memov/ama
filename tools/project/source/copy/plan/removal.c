/** @file
 * @brief Admit exact original removal material and review its joint survivor cut.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * prevents source or replacement removal from being authorized by a valid copy;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires removal observations to agree with captured preparation bytes.
 */
#include <string.h>
#include "internal.h"
/** Count exact removal rows, leaving duplicate projection rejection to the image gate. */
static size_t changes(const SourceCopyPlanInputs *inputs, const char *path) {
    size_t count = 0;
    for (size_t i = 0; i < inputs->change_count; i++) {
        const SourceImageChange *row = &inputs->changes[i];
        if (row->kind == SOURCE_IMAGE_REMOVE && row->entry.path && !strcmp(row->entry.path, path)) { count++; }
    }
    return count;
}
SourceCopyPlanStatus source_copy_plan_removal(SourceCopyPlan *plan) {
    const SourceCopyPlanInputs *inputs = &plan->inputs;
    const SourceRemovalSelection *selection = inputs->removal;
    if (selection && (!selection->valid || selection->tree->capture != inputs->observed->original
        || selection->analysis != inputs->copy->expansion.analysis)) { return SOURCE_COPY_PLAN_INVALID; }
    for (size_t i = 0; i < inputs->change_count; i++) {
        const SourceImageChange *change = &inputs->changes[i];
        if (change->kind != SOURCE_IMAGE_REMOVE) { continue; }
        const SourceRemovalSelectedFile *file = selection ? selection->selected : NULL;
        while (file && change->entry.path && strcmp(file->file->file.path, change->entry.path)) { file = file->next; }
        if (!file || !change->entry.path) { plan->failed_path = change->entry.path; return SOURCE_COPY_PLAN_REMOVAL; }
    }
    if (!selection) {
        plan->removal_status = source_removal_check(&plan->arena, inputs->copy->expansion.analysis->original.response.definitions,
            inputs->inputs, NULL, 0, NULL, 0, &plan->removal_review.consumers);
        if (plan->removal_status != SOURCE_REMOVAL_OK) { return SOURCE_COPY_PLAN_REMOVAL; }
        plan->removal_status = source_removal_client_roots(&plan->arena, inputs->copy->expansion.analysis->original.response.definitions,
            inputs->clients, NULL, 0, &plan->removal_review.clients);
        return plan->removal_status == SOURCE_REMOVAL_OK ? SOURCE_COPY_PLAN_OK : SOURCE_COPY_PLAN_REMOVAL;
    }
    for (const SourceRemovalSelectedFile *file = selection->selected; file; file = file->next) {
        const AcquisitionUnitEntry *old = file->file; const SourceCatalogImageEntry *row = NULL;
        plan->failed_path = old->file.path;
        if (changes(inputs, old->file.path) != 1) { return SOURCE_COPY_PLAN_REMOVAL; }
        for (size_t i = 0; i < inputs->observed->image.count; i++) {
            if (!strcmp(inputs->observed->image.entries[i].path, old->file.path)) { row = &inputs->observed->image.entries[i]; break; }
        }
        if (!row || !row->metadata_captured || (old->directory ? !S_ISDIR(row->metadata.st_mode) || !row->children_complete
            : !S_ISREG(row->metadata.st_mode) || !row->content || source_length(row->content) != source_length(old->file.content)
                || memcmp(source_bytes(row->content), source_bytes(old->file.content), source_length(row->content)))) { return SOURCE_COPY_PLAN_REMOVAL; }
    }
    plan->removal_status = source_removal_selection_review(inputs->removal, inputs->inputs, inputs->clients, &plan->removal_review);
    plan->failed_path = NULL;
    return plan->removal_status == SOURCE_REMOVAL_OK ? SOURCE_COPY_PLAN_OK : SOURCE_COPY_PLAN_REMOVAL;
}
