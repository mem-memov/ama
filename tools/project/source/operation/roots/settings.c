/** @file
 * @brief Match explicit root choices to final descriptors and real directory-copy provenance.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires carried roots to retain kind and unknown destination roots to be explicit;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not infer physical deletion from a removed setting.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** An actual directory row supplies carry provenance; no textual prefix guess suffices. */
static bool carried(const SourceFileMap *files, const char *old, const char *now) {
    for (const SourceFileMapping *row = files->first; row; row = row->next) {
        if (row->original->directory && !strcmp(row->original->file.path, old) && !strcmp(row->destination, now)) { return true; }
    }
    return false;
}
/** Report immutable descriptors; allocation failure retains the owned prefix. */
static SourceOperationRootsStatus append(SourceOperationRoots *owner, const ModuleRoot *old,
    const ModuleRoot *now, SourceOperationRootAction action, bool addition, bool explicit_destination) {
    if (old && now && old->kind != now->kind) { return SOURCE_OPERATION_ROOTS_KIND; }
    if (action == SOURCE_OPERATION_ROOT_KEEP && strcmp(old->path, now->path)) { return SOURCE_OPERATION_ROOTS_PLACEMENT; }
    if (old && action == SOURCE_OPERATION_ROOT_CARRY && !carried(owner->inputs.plan->inputs.files, old->path, now->path)) {
        return SOURCE_OPERATION_ROOTS_PLACEMENT;
    }
    SourceOperationRootSetting *row = source_arena_allocate(&owner->arena, sizeof(*row));
    if (!row) { return SOURCE_OPERATION_ROOTS_MEMORY; }
    *row = (SourceOperationRootSetting){.original = old, .final = now, .action = action,
        .addition = addition, .explicit_destination = explicit_destination};
    if (owner->last) { owner->last->next = row; } else { owner->first = row; } owner->last = row;
    return SOURCE_OPERATION_ROOTS_OK;
}
SourceOperationRootsStatus source_operation_roots_settings(SourceOperationRoots *owner) {
    const SourceOperationRootsInputs *inputs = &owner->inputs;
    const SourceOperationPlanInputs *plan = &inputs->plan->inputs;
    for (size_t i = 0; i < inputs->original_count; i++) {
        const SourceOperationRootDisposition *request = &inputs->originals[i];
        const ModuleRoot *old = &inputs->configured->roots[i]; owner->failed_path = old->path;
        if (request->action < SOURCE_OPERATION_ROOT_KEEP || request->action > SOURCE_OPERATION_ROOT_CARRY
            || (request->action == SOURCE_OPERATION_ROOT_DROP ? request->final != SIZE_MAX : request->final >= plan->final_root_count)) {
            return SOURCE_OPERATION_ROOTS_ACCOUNTING;
        }
        SourceOperationRootsStatus status = append(owner, old,
            request->action == SOURCE_OPERATION_ROOT_DROP ? NULL : &plan->final_roots[request->final], request->action, false, false);
        if (status != SOURCE_OPERATION_ROOTS_OK) { return status; }
    }
    for (size_t i = 0; i < inputs->addition_count; i++) {
        const SourceOperationRootAddition *request = &inputs->additions[i];
        if (request->final >= plan->final_root_count || (request->source != SIZE_MAX && request->source >= inputs->original_count)) {
            return SOURCE_OPERATION_ROOTS_ACCOUNTING;
        }
        const ModuleRoot *now = &plan->final_roots[request->final]; owner->failed_path = now->path;
        for (const SourceOperationRootSetting *row = owner->first; row; row = row->next) {
            if (row->final == now) { return SOURCE_OPERATION_ROOTS_ACCOUNTING; }
        }
        SourceOperationRootsStatus status = append(owner,
            request->source == SIZE_MAX ? NULL : &inputs->configured->roots[request->source], now,
            request->source == SIZE_MAX ? SOURCE_OPERATION_ROOT_ADD : SOURCE_OPERATION_ROOT_CARRY, true, request->source == SIZE_MAX);
        if (status != SOURCE_OPERATION_ROOTS_OK) { return status; }
    }
    for (size_t i = 0; i < plan->final_root_count; i++) {
        const SourceOperationRootSetting *row = owner->first;
        while (row && row->final != &plan->final_roots[i]) { row = row->next; }
        if (!row) { owner->failed_path = plan->final_roots[i].path; return SOURCE_OPERATION_ROOTS_ACCOUNTING; }
    }
    owner->failed_path = NULL; return SOURCE_OPERATION_ROOTS_OK;
}
