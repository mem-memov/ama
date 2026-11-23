/** @file
 * @brief Construct one closed typed write set before source-image or final-operation admission.
 * [TOOL-04](../../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids discovering additional write obligations during publication.
 */
#include "internal.h"
#include <stdint.h>
/** Checked total upper bound covers D, three initializer rows and each config update. */
static SourceOperationWritesStatus reserve(SourceOperationWrites *writes) {
    if (writes->inputs.module_count > SIZE_MAX / sizeof(SourceOperationPreparedWrite)) { return SOURCE_OPERATION_WRITES_LIMIT; }
    size_t count = writes->inputs.removal ? writes->inputs.removal->path_count : 0;
    if (writes->inputs.initialization_count > (SIZE_MAX - count) / 3) { return SOURCE_OPERATION_WRITES_LIMIT; }
    count += writes->inputs.initialization_count * 3;
    if (writes->inputs.configuration_count > SIZE_MAX - count) { return SOURCE_OPERATION_WRITES_LIMIT; }
    count += writes->inputs.configuration_count;
    if (count > SIZE_MAX / sizeof(*writes->changes) || count > SIZE_MAX / sizeof(*writes->owners)) { return SOURCE_OPERATION_WRITES_LIMIT; }
    writes->capacity = count;
    writes->changes = count ? source_arena_allocate(&writes->arena, count * sizeof(*writes->changes)) : NULL;
    writes->owners = count ? source_arena_allocate(&writes->arena, count * sizeof(*writes->owners)) : NULL;
    return count && (!writes->changes || !writes->owners) ? SOURCE_OPERATION_WRITES_MEMORY : SOURCE_OPERATION_WRITES_OK;
}
SourceOperationWritesStatus source_operation_writes_build(SourceOperationWrites *writes, const SourceOperationWritesInputs *inputs) {
    if (!writes || writes->attempted) { return SOURCE_OPERATION_WRITES_INVALID; }
    writes->attempted = true; writes->status = source_operation_writes_admit(inputs);
    if (writes->status != SOURCE_OPERATION_WRITES_OK) { return writes->status; }
    writes->inputs = *inputs; writes->status = reserve(writes);
    if (writes->status != SOURCE_OPERATION_WRITES_OK) { return writes->status; }
    if (inputs->removal) {
        SourceRemovalMaterialStatus material = source_removal_material_prepare(&writes->arena, inputs->removal, &writes->removal_material);
        if (material != SOURCE_REMOVAL_MATERIAL_OK) { return writes->status = material == SOURCE_REMOVAL_MATERIAL_MEMORY
            ? SOURCE_OPERATION_WRITES_MEMORY : SOURCE_OPERATION_WRITES_MATERIAL; }
        for (size_t i = 0; i < writes->removal_material.count; i++) {
            SourceOperationExtraOwner owner = {.kind = SOURCE_OPERATION_EXTRA_REMOVAL, .owner.removal = inputs->removal};
            writes->status = source_operation_writes_extra(writes, writes->removal_material.changes[i], owner);
            if (writes->status != SOURCE_OPERATION_WRITES_OK) { return writes->status; }
        }
    }
    writes->status = source_operation_writes_initializations(writes);
    if (writes->status != SOURCE_OPERATION_WRITES_OK) { return writes->status; }
    for (const SourceGraphPreparedText *main = inputs->main_texts; main; main = main->next) {
        SourceOperationPreparedWrite proposed = {.kind = SOURCE_OPERATION_PREPARED_MAIN, .text = {.edits = main->edits}, .owner.main = main};
        writes->status = source_operation_writes_text(writes, &proposed);
        if (writes->status != SOURCE_OPERATION_WRITES_OK) { return writes->status; }
    }
    writes->status = source_operation_writes_modules(writes);
    if (writes->status == SOURCE_OPERATION_WRITES_OK) { writes->status = source_operation_writes_configurations(writes); }
    if (writes->status == SOURCE_OPERATION_WRITES_OK) { writes->valid = true; writes->failed_path = NULL; }
    return writes->status;
}
