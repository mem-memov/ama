/** @file
 * @brief Publish immutable certificate state only after actual final graph, client and rebuild admission.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never treats a prepared write recipe or compiling wrong image as source-operation proof.
 */
#include "complete.h"
SourceOperationCompleteStatus source_operation_complete_seal(SourceOperationComplete *owner, const SourceCompilerClient *compiler) {
    if (!owner || !owner->prepared || owner->seal_attempted) { return SOURCE_OPERATION_COMPLETE_INVALID; }
    owner->seal_attempted = true; owner->compiler = compiler;
    owner->proof_status = source_operation_plan_prove(&owner->plan, compiler, &owner->proof_issue);
    if (owner->proof_status != SOURCE_COPY_PROOF_OK) { return owner->status = SOURCE_OPERATION_COMPLETE_PROOF; }
    if (source_operation_clients_build(&owner->clients, &owner->plan, compiler) != SOURCE_OPERATION_CLIENTS_OK) {
        return owner->status = SOURCE_OPERATION_COMPLETE_CLIENTS;
    }
    if (source_operation_rebuild_build(&owner->rebuild, &owner->roots, &owner->clients) != SOURCE_OPERATION_REBUILD_OK) {
        return owner->status = SOURCE_OPERATION_COMPLETE_REBUILD;
    }
    owner->sealed = true; return owner->status = SOURCE_OPERATION_COMPLETE_OK;
}
