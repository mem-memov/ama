/**
 * @file
 * @brief Validate host origin before interpreting context-local instance identities.
 *
 * [REF-02](../../../spec/language/values-and-references.md#ref-02) rejects equal local triples
 * from another context. [REF-07](../../../spec/language/values-and-references.md#ref-07)
 * separates forming a checked reference from acquiring or executing its target.
 */
#include "internal.h"
#include "runtime/objects/internal.h"

/** Wrap a trusted local published identity after exact pool validation. No availability check
 * prevents forming a reference to the currently running instance; no target resource is acquired.
 */
AmaStatus ama_context_reference(const AmaContextLease *lease, AmaHandle local,
    AmaContextReference *reference) {
    AmaContext *context = ama_context_leased(lease);
    if (context == NULL || reference == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (context->world.failed || context->status.code != AMA_OK) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_object_lookup(&context->world, local, false, &type, &cell);
    if (status.code == AMA_OK) { *reference = (AmaContextReference){context->identity, local}; }
    return status;
}

/** Reject a foreign context before any local registry/pool read, even when the triple would
 * successfully select a different live object here. Publish borrowed outputs only after success.
 */
AmaStatus ama_context_resolve(const AmaContextLease *lease, AmaContextReference reference,
    AmaAccessMode mode, AmaObjectType **type, AmaPoolCell **cell) {
    AmaContext *context = ama_context_leased(lease);
    if (context == NULL || type == NULL || cell == NULL
        || mode < AMA_ACCESS_ENTRY || mode > AMA_ACCESS_READ) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (reference.context == 0) { return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0); }
    if (reference.context != context->identity) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (context->world.failed || context->status.code != AMA_OK) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaObjectType *selected_type = NULL;
    AmaPoolCell *selected_cell = NULL;
    AmaStatus status = ama_object_lookup(&context->world, reference.local, false, &selected_type, &selected_cell);
    if (status.code == AMA_OK) { status = ama_instance_access(&selected_cell->metadata, mode); }
    if (status.code == AMA_OK) { *type = selected_type; *cell = selected_cell; }
    return status;
}
