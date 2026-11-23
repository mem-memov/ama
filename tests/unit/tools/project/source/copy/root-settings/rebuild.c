/** @file
 * @brief Verify owned rebuild intentions cover all actual original/final domains without artifact mutation.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/rebuild/rebuild.h"
/** Check ordered provenance and every failed allocation of the domain report.
 * Unknown output paths are intentionally absent; no generated file is inspected. */
static void check_operation_rebuild_intentions(const SourceOperationRoots *roots, const SourceCompilerClient *compiler) {
    SourceOperationClients clients; source_operation_clients_init(&clients, sources.allocator);
    assert(source_operation_clients_build(&clients, roots->inputs.plan, compiler) == SOURCE_OPERATION_CLIENTS_OK);
    const SourceRootCapture *original = roots->inputs.plan->inputs.observed->original;
    const ModuleRoot *expected[] = {roots->inputs.configured->roots, original->roots, roots->inputs.plan->inputs.final_roots};
    size_t counts[] = {roots->inputs.configured->root_count, original->root_count, roots->inputs.plan->inputs.final_root_count};
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceOperationRebuild rebuild; source_operation_rebuild_init(&rebuild, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceOperationRebuildStatus status = source_operation_rebuild_build(&rebuild, roots, &clients);
        if (!failure) {
            assert(status == SOURCE_OPERATION_REBUILD_OK && rebuild.valid);
            total = allocation_count - before; assert(total);
            assert(rebuild.count == counts[0] + counts[1] + counts[2] && rebuild.classes == 31 && rebuild.boundaries == 7);
            const SourceOperationRebuildScope *row = rebuild.first;
            for (size_t domain = 0; domain < 3; domain++) {
                for (size_t index = 0; index < counts[domain]; index++) {
                    assert(row && row->root == &expected[domain][index] && row->domain == (SourceOperationRebuildDomain)domain);
                    row = row->next;
                }
            }
            assert(!row && rebuild.last && !rebuild.last->next);
            size_t calls = allocation_count;
            assert(source_operation_rebuild_build(&rebuild, roots, &clients) == SOURCE_OPERATION_REBUILD_INVALID && rebuild.valid && allocation_count == calls);
        } else { assert(status == SOURCE_OPERATION_REBUILD_MEMORY && !rebuild.valid && !rebuild.classes); }
        failure_at = 0; source_operation_rebuild_destroy(&rebuild); assert(live_allocations == baseline);
    }
    for (unsigned absent = 0; absent < 2; absent++) {
        SourceOperationRebuild rebuild; source_operation_rebuild_init(&rebuild, sources.allocator);
        assert(source_operation_rebuild_build(&rebuild, absent ? NULL : roots, absent ? &clients : NULL) == SOURCE_OPERATION_REBUILD_INVALID && !rebuild.valid);
        source_operation_rebuild_destroy(&rebuild);
    }
    SourceOperationPlan other; source_operation_plan_init(&other, sources.allocator);
    assert(source_operation_plan_build(&other, &roots->inputs.plan->inputs) == SOURCE_OPERATION_PLAN_OK);
    SourceOperationClients foreign; source_operation_clients_init(&foreign, sources.allocator);
    assert(source_operation_clients_build(&foreign, &other, compiler) == SOURCE_OPERATION_CLIENTS_OK);
    SourceOperationRebuild rebuild; source_operation_rebuild_init(&rebuild, sources.allocator);
    assert(source_operation_rebuild_build(&rebuild, roots, &foreign) == SOURCE_OPERATION_REBUILD_INVALID && !rebuild.valid);
    source_operation_rebuild_destroy(&rebuild); source_operation_clients_destroy(&foreign);
    source_operation_plan_destroy(&other); source_operation_clients_destroy(&clients);
    assert(source_operation_rebuild_build(NULL, roots, NULL) == SOURCE_OPERATION_REBUILD_INVALID);
}
