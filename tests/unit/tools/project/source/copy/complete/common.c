/** @file
 * @brief Compose typed writers and real original/final operation evidence through the single complete owner.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#ifndef AMA_SOURCE_COMPLETE_TEST_COMMON
#define AMA_SOURCE_COMPLETE_TEST_COMMON
#include "../../../../../../../tools/project/source/operation/complete/complete.h"
/** Form explicit fixture settings, not production defaults: these independent
 * accepted fixtures have one final root and at most two original roots. */
static SourceOperationCompleteInputs complete_fixture_inputs(const SourceOperationWritesInputs *writes,
    const SourceOperationPlan *reference, const SourceOperationIntent *intent, SourceOperationRootDisposition *roots) {
    const SourceRootCapture *original = reference->inputs.observed->original;
    assert(original->root_count <= 2 && reference->inputs.final_root_count == 1);
    for (size_t i = 0; i < original->root_count; i++) {
        roots[i] = (SourceOperationRootDisposition){
            !strcmp(original->roots[i].path, reference->inputs.final_roots[0].path)
                ? SOURCE_OPERATION_ROOT_KEEP : SOURCE_OPERATION_ROOT_CARRY, 0};
    }
    return (SourceOperationCompleteInputs){.writes = writes, .intent = intent,
        .original = reference->inputs.original, .configured = original,
        .final_roots = reference->inputs.final_roots, .final_root_count = 1,
        .contexts = reference->inputs.contexts, .inputs = reference->inputs.inputs,
        .clients = reference->inputs.clients, .root_originals = roots, .root_original_count = original->root_count};
}
/** Independent final compilation already succeeded; the complete join must
 * connect its actual bytes to intent, writes, exact sets and root/client facts. */
static void check_complete_operation(const SourceOperationWritesInputs *writes, const SourceOperationPlan *reference,
    const SourceOperationIntent *intent, const SourceCompilerClient *compiler) {
    SourceOperationRootDisposition roots[2];
    SourceOperationCompleteInputs inputs = complete_fixture_inputs(writes, reference, intent, roots);
    SourceOperationComplete owner; source_operation_complete_init(&owner, sources.allocator);
    assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_OK && owner.prepared && !owner.sealed);
    assert(owner.plan.image == owner.plan.capture.image && owner.configuration_count == 1
        && owner.authorities[0].kind == SOURCE_OPERATION_SCOPE_WRITER && owner.material.valid && owner.roots.valid);
    assert(source_operation_complete_seal(&owner, compiler) == SOURCE_OPERATION_COMPLETE_OK && owner.sealed && owner.clients.valid && owner.rebuild.valid);
    size_t calls = allocation_count;
    assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_INVALID && owner.sealed && allocation_count == calls);
    assert(source_operation_complete_seal(&owner, compiler) == SOURCE_OPERATION_COMPLETE_INVALID && owner.sealed && allocation_count == calls);
    source_operation_complete_destroy(&owner);
    size_t baseline = live_allocations, total = 0;
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
    for (unsigned profile = 0; profile < 4; profile++) {
        SourceOperationCompleteInputs invalid = inputs; SourceOperationWritesInputs unowned = *writes;
        if (!profile) { invalid.root_originals = NULL; }
        if (profile == 1) { unowned.configurations = NULL; unowned.configuration_count = 0; invalid.writes = &unowned; }
        if (profile == 2) { invalid.original = NULL; }
        source_operation_complete_init(&owner, sources.allocator);
        SourceOperationCompleteStatus expected = !profile ? SOURCE_OPERATION_COMPLETE_ROOTS
            : profile == 1 ? SOURCE_OPERATION_COMPLETE_CONFIGURATION : profile == 2 ? SOURCE_OPERATION_COMPLETE_INVALID : SOURCE_OPERATION_COMPLETE_OK;
        assert(source_operation_complete_prepare(&owner, &invalid) == expected && !owner.sealed);
        if (profile == 3) {
            assert(source_operation_complete_seal(&owner, &inputs.original->original) == SOURCE_OPERATION_COMPLETE_PROOF && !owner.sealed);
        }
        source_operation_complete_destroy(&owner); assert(live_allocations == baseline);
    }
    SourceClientRoot missing = {.definition = inputs.original->original.response.definitions,
        .kind = SOURCE_CLIENT_ENTRY, .action = SOURCE_CLIENT_MAP, .destination = "/vendor/missing/v1.0.0/src/missing"};
    inputs.clients = &missing;
    source_operation_complete_init(&owner, sources.allocator);
    assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_OK);
    assert(source_operation_complete_seal(&owner, compiler) == SOURCE_OPERATION_COMPLETE_CLIENTS
        && owner.clients.status == SOURCE_OPERATION_CLIENTS_MISSING && owner.clients.failed == &missing && !owner.sealed);
    source_operation_complete_destroy(&owner); assert(live_allocations == baseline);
    assert(source_operation_complete_prepare(NULL, &inputs) == SOURCE_OPERATION_COMPLETE_INVALID);
    assert(source_operation_complete_seal(NULL, compiler) == SOURCE_OPERATION_COMPLETE_INVALID);
}
#endif
