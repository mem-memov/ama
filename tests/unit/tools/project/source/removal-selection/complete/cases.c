/** @file
 * @brief Seal genuine deletion and supplier-reselection operations through the same complete owner as copies.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/complete/complete.h"
/** No copy closure or synthetic valid owner stands in for deletion. All root
 * settings remain explicit, all final scopes come from real plan discovery. */
static void check_complete_removal(const SourceOperationPlan *reference, const SourceCompilerClient *compiler,
    bool failures, bool supplier) {
    assert(!reference->inputs.copy && reference->inputs.removal);
    const SourceRootCapture *configured = reference->inputs.observed->original;
    assert(configured->root_count <= 3 && configured->root_count == reference->inputs.final_root_count);
    SourceOperationRootDisposition roots[3];
    for (size_t i = 0; i < configured->root_count; i++) { roots[i] = (SourceOperationRootDisposition){SOURCE_OPERATION_ROOT_KEEP, i}; }
    SourceOperationIntent intent = {.kind = SOURCE_OPERATION_DELETE,
        .source = reference->inputs.removal->tree, .source_removal = reference->inputs.removal};
    SourceOperationWritesInputs writes = {.observed = reference->inputs.observed, .mains = reference->inputs.mains,
        .files = reference->inputs.files, .removal = reference->inputs.removal};
    SourceOperationCompleteInputs inputs = {.writes = &writes, .intent = &intent, .original = reference->inputs.original,
        .configured = configured, .final_roots = reference->inputs.final_roots, .final_root_count = configured->root_count,
        .inputs = reference->inputs.inputs, .clients = reference->inputs.clients,
        .root_originals = roots, .root_original_count = configured->root_count};
    SourceOperationComplete owner; source_operation_complete_init(&owner, sources.allocator);
    assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_OK && owner.prepared && !owner.sealed);
    bool has_supplier = false;
    for (size_t i = 0; i < owner.configuration_count; i++) {
        assert(owner.authorities[i].kind == SOURCE_OPERATION_SCOPE_PRESERVED || owner.authorities[i].kind == SOURCE_OPERATION_SCOPE_SUPPLIER);
        has_supplier |= owner.authorities[i].kind == SOURCE_OPERATION_SCOPE_SUPPLIER;
    }
    assert(has_supplier == supplier);
    assert(source_operation_complete_seal(&owner, compiler) == SOURCE_OPERATION_COMPLETE_OK && owner.sealed && owner.rebuild.valid);
    source_operation_complete_destroy(&owner);
    if (!failures) { return; }
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_operation_complete_init(&owner, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceOperationCompleteStatus status = source_operation_complete_prepare(&owner, &inputs);
        if (!failure) { assert(status == SOURCE_OPERATION_COMPLETE_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_OPERATION_COMPLETE_OK && !owner.prepared && !owner.sealed); }
        failure_at = 0; source_operation_complete_destroy(&owner); assert(live_allocations == baseline);
    }
    total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_operation_complete_init(&owner, sources.allocator);
        assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_OK);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceOperationCompleteStatus status = source_operation_complete_seal(&owner, compiler);
        if (!failure) { assert(status == SOURCE_OPERATION_COMPLETE_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_OPERATION_COMPLETE_OK && owner.prepared && !owner.sealed); }
        failure_at = 0; source_operation_complete_destroy(&owner); assert(live_allocations == baseline);
    }
}
