/** @file
 * @brief Keep explicit entry/export choices distinct from successful final source compilation.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/clients/clients.h"
/** Build genuine shared plans for every disposition, including a missing target
 * in an otherwise accepted exact final compiler reply. Never mark mock owners valid. */
static void check_operation_client_settings(const SourceOperationPlan *reference, const SourceCompilerClient *compiler) {
    const DeliverySourceDefinition *original = reference->inputs.original->original.response.definitions;
    assert(original && compiler->response.definitions);
    const DeliverySourceDefinition *target = compiler->response.definitions;
    const char *missing = "/vendor/absent/v1.0.0/src/absent";
    for (unsigned profile = 0; profile < 5; profile++) {
        SourceClientRoot roots[2] = {{.definition = original, .kind = SOURCE_CLIENT_ENTRY},
            {.definition = original, .kind = SOURCE_CLIENT_EXPORT}};
        roots[0].next = &roots[1];
        roots[0].action = roots[1].action = SOURCE_CLIENT_MAP;
        roots[0].destination = roots[1].destination = target->identity;
        if (profile == 1) { roots[0].action = SOURCE_CLIENT_DROP; roots[0].destination = NULL; }
        if (profile == 2) { roots[0].destination = missing; }
        if (profile == 3) { roots[0].next = NULL; roots[0].action = SOURCE_CLIENT_KEEP; roots[0].destination = NULL; }
        SourceOperationPlanInputs inputs = reference->inputs;
        inputs.clients = profile == 4 ? NULL : roots;
        SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
        SourceOperationPlanStatus built = source_operation_plan_build(&plan, &inputs);
        bool removed = false;
        if (inputs.removal) {
            for (size_t i = 0; i < inputs.removal->identity_count; i++) {
                if (!strcmp(inputs.removal->identities[i], original->identity)) { removed = true; }
            }
        }
        if (profile == 3 && removed) {
            assert(built == SOURCE_OPERATION_PLAN_REMOVAL && plan.removal_status == SOURCE_REMOVAL_BLOCKED);
            source_operation_plan_destroy(&plan); continue;
        }
        assert(built == SOURCE_OPERATION_PLAN_OK);
        SourceOperationClients report; source_operation_clients_init(&report, sources.allocator);
        SourceOperationClientsStatus expected = profile == 2 ? SOURCE_OPERATION_CLIENTS_MISSING : SOURCE_OPERATION_CLIENTS_OK;
        assert(source_operation_clients_build(&report, &plan, compiler) == expected);
        if (profile == 2) { assert(!report.valid && report.failed == roots); }
        else if (profile == 4) { assert(report.valid && !report.first && !report.last); }
        else {
            assert(report.valid && report.first->request == roots);
            if (profile == 1) { assert(!report.first->final); }
            else { assert(report.first->final && !strcmp(report.first->final->identity,
                profile == 3 ? original->identity : target->identity)); }
            if (profile != 3) { assert(report.first->next == report.last && report.last->request == &roots[1] && report.last->final == target); }
            size_t before = allocation_count;
            assert(source_operation_clients_build(&report, &plan, compiler) == SOURCE_OPERATION_CLIENTS_INVALID && report.valid && allocation_count == before);
        }
        source_operation_clients_destroy(&report);
        if (!profile) {
            size_t total = 0, baseline = live_allocations;
            for (size_t failure = 0; !failure || failure <= total; failure++) {
                source_operation_clients_init(&report, sources.allocator);
                size_t before = allocation_count; if (failure) { failure_at = before + failure; }
                SourceOperationClientsStatus status = source_operation_clients_build(&report, &plan, compiler);
                if (!failure) { assert(status == SOURCE_OPERATION_CLIENTS_OK); total = allocation_count - before; assert(total); }
                else { assert(status == SOURCE_OPERATION_CLIENTS_MEMORY && !report.valid); }
                failure_at = 0; source_operation_clients_destroy(&report); assert(live_allocations == baseline);
            }
        }
        source_operation_plan_destroy(&plan);
    }
    SourceOperationClients report; source_operation_clients_init(&report, sources.allocator);
    assert(source_operation_clients_build(&report, reference, NULL) == SOURCE_OPERATION_CLIENTS_INVALID && !report.valid);
    source_operation_clients_destroy(&report);
    SourceClientRoot cyclic = {.definition = original, .kind = SOURCE_CLIENT_ENTRY}; cyclic.next = &cyclic;
    SourceOperationPlanInputs malformed = reference->inputs; malformed.clients = &cyclic;
    SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &malformed) == SOURCE_OPERATION_PLAN_REMOVAL
        && plan.removal_status == SOURCE_REMOVAL_INVALID && !plan.prepared);
    source_operation_plan_destroy(&plan);
    assert(source_operation_clients_build(NULL, reference, compiler) == SOURCE_OPERATION_CLIENTS_INVALID);
}
