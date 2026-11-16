/** @file
 * @brief Rebuild discovery, original binding and final staging under held directory leases before any mutation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * checks the complete current scope, including new/deleted sources and native companion presence.
 */
#include "../internal.h"

/** Temporary compiler owners are destroyed in reverse order on every path; diagnostics never borrow them after return. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_revalidate(RelocationTransaction *transaction) {
    const char *source = transaction->staging->references->preview->source;
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_REVALIDATE,source)) { return false; }
    RelocationRequest discovery;
    RelocationStagingRequest settings;
    if (!relocation_transaction_request(transaction,&discovery,&settings)) { return false; }
    RelocationPreview preview;
    RelocationReferences references;
    RelocationStaging staging;
    relocation_preview_init(&preview,transaction->arena.allocator);
    relocation_references_init(&references,transaction->arena.allocator);
    relocation_staging_init(&staging,transaction->arena.allocator);
    settings.references = &references;
    bool complete = relocation_preview_build(&preview,&discovery)
        && relocation_references_build(&references,&preview) && relocation_staging_build(&staging,&settings);
    if (!complete) {
        RelocationStatus cause = !preview.ready ? preview.status : !references.ready ? references.status : staging.status;
        relocation_transaction_fail(transaction,cause == RELOCATION_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY
            : RELOCATION_CHANGED_INPUT,"The current project could not reproduce the validated relocation plan",source);
    } else { complete = relocation_transaction_same_plan(transaction,&staging); }
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    relocation_preview_destroy(&preview);
    return complete;
}
