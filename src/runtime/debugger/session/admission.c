/** @file
 * @brief Hold one checked exclusive context lease while inspecting state at rest.
 * [PAR-02](../../../../spec/language/async-and-parallelism.md#par-02) rejects concurrent entry;
 * [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) preserves terminal diagnostics without reconstructed objects.
 */
#include <stdint.h>
#include "../internal.h"
/** Revalidate the lease each time; cached views alone never grant access after release. */
AmaStatus ama_debug_validate(const AmaDebugSession *session) {
    if (session == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    AmaContextView view = {0};
    AmaStatus status = ama_context_inspect(&session->lease,&view);
    if (status.code != AMA_OK) { return status; }
    if (view.world != session->view.world || view.program != session->view.program || view.control != session->view.control) {
        return ama_status_make(AMA_INVALID_STATE,NULL,0);
    }
    if (view.control->phase == AMA_RUNTIME_EXECUTING || view.world->operation_active) { return ama_status_make(AMA_BUSY,NULL,0); }
    return ama_status_success();
}
/** Failed admission releases only its private candidate lease and leaves caller storage unchanged. */
AmaStatus ama_debug_begin(AmaContextRegistry *registry, uint64_t identity, const AmaDebugCatalog *catalog,
    const AmaDebugNativeReader *readers, size_t count, AmaDebugSession *output) {
    if (output == NULL || output->lease.context != NULL || output->lease.registry != NULL || output->lease.identity != 0
        || output->view.world != NULL || output->view.program != NULL || output->view.control != NULL
        || output->catalog != NULL || output->readers != NULL || output->reader_count != 0 || output->visits != 0
        || (count != 0 && readers == NULL)) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    if (count > SIZE_MAX/sizeof(*readers)) { return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }
    AmaDebugSession candidate = {.catalog=catalog,.readers=readers,.reader_count=count};
    AmaStatus status = ama_context_enter(registry,identity,&candidate.lease);
    if (status.code != AMA_OK) { return status; }
    status = ama_context_inspect(&candidate.lease,&candidate.view);
    if (status.code == AMA_OK) { status = ama_debug_validate(&candidate); }
    if (status.code == AMA_OK && candidate.view.control->phase != AMA_RUNTIME_TERMINATED) { status = ama_debug_schema(&candidate); }
    if (status.code != AMA_OK) { (void)ama_context_leave(&candidate.lease); return status; }
    *output = candidate;
    return ama_status_success();
}
/** End clears every borrowed view before releasing the gate; callbacks cannot end their supplying session. */
AmaStatus ama_debug_end(AmaDebugSession *session) {
    AmaStatus status = ama_debug_validate(session);
    if (status.code != AMA_OK) { return status; }
    if (session->visits != 0) { return ama_status_make(AMA_BUSY,NULL,0); }
    AmaContextLease lease = session->lease;
    *session = (AmaDebugSession){0};
    return ama_context_leave(&lease);
}
/** Nested read queries are allowed; finite caller frames cannot release their borrowed owner early. */
AmaStatus ama_debug_visit_begin(AmaDebugSession *session) {
    AmaStatus status = ama_debug_validate(session);
    if (status.code != AMA_OK) { return status; }
    if (session->visits == SIZE_MAX) { return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }
    session->visits++;
    return ama_status_success();
}
/** No user/native hook is involved in retiring this read-only frame credit. */
void ama_debug_visit_end(AmaDebugSession *session) { session->visits--; }
