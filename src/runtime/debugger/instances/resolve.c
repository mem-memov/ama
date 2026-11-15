/** @file
 * @brief Authenticate context origin and actual live pool identity independently of ordinary instance availability.
 * [REF-02](../../../../spec/language/values-and-references.md#ref-02) checks origin before local triples;
 * [RT-04](../../../../spec/implementation/runtime.md#rt-04) keeps metadata separate from payload access.
 */
#include "../internal.h"
/** Busy activity is inspectable under exclusive context admission; native/read execution is still separately gated. */
AmaStatus ama_debug_resolve(const AmaDebugSession *session, AmaContextReference reference,
    const AmaProgramType **type, AmaPoolCell **cell) {
    AmaStatus status = ama_debug_validate(session);
    if (status.code != AMA_OK) { return status; }
    if (type == NULL || cell == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    if (reference.context == 0) { return ama_status_make(AMA_EMPTY_REFERENCE,NULL,0); }
    if (reference.context != session->lease.identity) { return ama_status_make(AMA_TYPE_MISMATCH,NULL,0); }
    if (session->view.world->failed || session->view.control->phase == AMA_RUNTIME_TERMINATED) {
        return ama_status_make(AMA_INVALID_STATE,NULL,0);
    }
    if (ama_handle_is_empty(reference.local)) { return ama_status_make(AMA_EMPTY_REFERENCE,NULL,0); }
    if (reference.local.type_id == 0) { return ama_status_make(AMA_TYPE_MISMATCH,NULL,0); }
    const AmaProgramType *selected = ama_program_type(session->view.program,reference.local.type_id);
    if (selected == NULL) { return ama_status_make(AMA_TYPE_MISMATCH,NULL,0); }
    AmaPoolCell *selected_cell = NULL;
    status = ama_pool_lookup(&selected->object->pool,reference.local,false,&selected_cell);
    if (status.code != AMA_OK) { return status; }
    if (selected_cell->metadata.lifetime.phase != AMA_LIFETIME_LIVE) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    *type = selected;
    *cell = selected_cell;
    return ama_status_success();
}
