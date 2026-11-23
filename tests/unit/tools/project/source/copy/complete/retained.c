/** @file
 * @brief Seal root-preserving operations with initialized/existing package permissions and module-alias recipes.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/complete/complete.h"
/** These fixtures use exact existing roots and a selected single main. The real
 * raw placement still must agree with P and the complete source/test closure. */
static void check_complete_retained_roots(const SourceOperationWritesInputs *writes,
    const SourceCopyContextOmissions *contexts, const SourceCompilerClient *compiler, SourceOperationScopeOrigin expected_kind) {
    const SourceRootCapture *original = writes->observed->original;
    assert(writes->copy && original->root_count <= 3);
    const SourceFileMapping *raw = writes->files->first;
    while (raw && strcmp(raw->source, writes->copy->tree->operand.path)) { raw = raw->next; }
    assert(raw);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, writes->copy->tree, raw->destination) == SOURCE_PLACEMENT_OK);
    SourceOperationIntent intent = {.kind = SOURCE_OPERATION_COPY, .source = writes->copy->tree, .placement = &placement};
    SourceOperationRootDisposition roots[3];
    for (size_t i = 0; i < original->root_count; i++) { roots[i] = (SourceOperationRootDisposition){SOURCE_OPERATION_ROOT_KEEP, i}; }
    SourceOperationCompleteInputs inputs = {.writes = writes, .intent = &intent, .original = writes->copy->expansion.analysis,
        .configured = original, .final_roots = original->roots, .final_root_count = original->root_count,
        .contexts = contexts, .root_originals = roots, .root_original_count = original->root_count};
    SourceOperationComplete owner; source_operation_complete_init(&owner, sources.allocator);
    SourceOperationCompleteStatus prepared = source_operation_complete_prepare(&owner, &inputs);
    if (prepared != SOURCE_OPERATION_COMPLETE_OK) {
        fprintf(stderr, "complete retained: stage=%d plan=%d material=%d path=%s\n", (int)prepared,
            (int)owner.plan.status, (int)owner.material.status, owner.failed_path ? owner.failed_path : "none");
    }
    assert(prepared == SOURCE_OPERATION_COMPLETE_OK && owner.prepared && !owner.sealed);
    bool found = false;
    for (size_t i = 0; i < owner.configuration_count; i++) { found |= owner.authorities[i].kind == expected_kind; }
    assert(found);
    assert(source_operation_complete_seal(&owner, compiler) == SOURCE_OPERATION_COMPLETE_OK && owner.sealed);
    source_operation_complete_destroy(&owner);
    if (writes->module_count) {
        SourceOperationWritesInputs missing = *writes; missing.modules = NULL; missing.module_count = 0; inputs.writes = &missing;
        source_operation_complete_init(&owner, sources.allocator);
        assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_CONFIGURATION
            && !owner.prepared && !owner.sealed && owner.failed_path);
        source_operation_complete_destroy(&owner); inputs.writes = writes;
    }
    if (writes->initialization_count) {
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            source_operation_complete_init(&owner, sources.allocator);
            size_t before = allocation_count; if (failure) { failure_at = before + failure; }
            SourceOperationCompleteStatus status = source_operation_complete_prepare(&owner, &inputs);
            if (!failure) { assert(status == SOURCE_OPERATION_COMPLETE_OK); total = allocation_count - before; assert(total); }
            else { assert(status != SOURCE_OPERATION_COMPLETE_OK && !owner.prepared); }
            failure_at = 0; source_operation_complete_destroy(&owner); assert(live_allocations == baseline);
        }
    }
    source_tree_placement_destroy(&placement);
}
