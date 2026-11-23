/** @file
 * @brief Join explicit client dispositions with the exact original removal cut and final registry.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * records client choices before publication; a successful final compilation alone
 * does not prove that a selected entry/export still exists.
 */
#include "clients.h"
#include <string.h>
/** Reuse original-cut semantics instead of inventing an independent client waiver. */
static SourceOperationClientsStatus originals(SourceOperationClients *owner) {
    const SourceOperationPlanInputs *inputs = &owner->plan->inputs;
    SourceRemovalStatus status = source_removal_client_roots(&owner->arena,
        inputs->original->original.response.definitions, inputs->clients,
        inputs->removal ? inputs->removal->identities : NULL,
        inputs->removal ? inputs->removal->identity_count : 0, &owner->blockers);
    if (status == SOURCE_REMOVAL_OK) { return SOURCE_OPERATION_CLIENTS_OK; }
    if (status == SOURCE_REMOVAL_BLOCKED) { return SOURCE_OPERATION_CLIENTS_BLOCKED; }
    return status == SOURCE_REMOVAL_OUT_OF_MEMORY ? SOURCE_OPERATION_CLIENTS_MEMORY : SOURCE_OPERATION_CLIENTS_INVALID;
}
SourceOperationClientsStatus source_operation_clients_build(SourceOperationClients *owner,
    const SourceOperationPlan *plan, const SourceCompilerClient *compiler) {
    if (!owner || owner->attempted) { return SOURCE_OPERATION_CLIENTS_INVALID; }
    owner->attempted = true; owner->status = SOURCE_OPERATION_CLIENTS_INVALID;
    if (!plan || !plan->prepared || !compiler || !compiler->valid || !plan->inputs.original
        || !plan->inputs.original->valid) { return owner->status; }
    owner->plan = plan; owner->compiler = compiler;
    if (!delivery_source_response_matches(&compiler->response, &plan->capture.view)) {
        return owner->status = SOURCE_OPERATION_CLIENTS_INPUTS;
    }
    owner->status = originals(owner);
    if (owner->status != SOURCE_OPERATION_CLIENTS_OK) { return owner->status; }
    for (const SourceClientRoot *request = plan->inputs.clients; request; request = request->next) {
        const DeliverySourceDefinition *final = NULL;
        if (request->action != SOURCE_CLIENT_DROP) {
            const char *identity = request->action == SOURCE_CLIENT_MAP ? request->destination : request->definition->identity;
            final = compiler->response.definitions;
            while (final && strcmp(final->identity, identity)) { final = final->next; }
            if (!final) { owner->failed = request; return owner->status = SOURCE_OPERATION_CLIENTS_MISSING; }
        }
        SourceOperationClientSetting *row = source_arena_allocate(&owner->arena, sizeof(*row));
        if (!row) { return owner->status = SOURCE_OPERATION_CLIENTS_MEMORY; }
        *row = (SourceOperationClientSetting){.request = request, .final = final};
        if (owner->last) { owner->last->next = row; } else { owner->first = row; }
        owner->last = row;
    }
    owner->valid = true; return owner->status = SOURCE_OPERATION_CLIENTS_OK;
}
