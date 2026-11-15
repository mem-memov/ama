/** @file
 * @brief Copy real admitted context status/control without implying retained instance history after cleanup.
 * [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) separates current metadata from history.
 */
#include "../internal.h"
/** Empty stored handles remain canonical empty host references; nonempty handles retain this exact context origin. */
AmaContextReference ama_debug_reference(const AmaDebugSession *session, AmaHandle handle) {
    return ama_handle_is_empty(handle) ? (AmaContextReference){0} : (AmaContextReference){session->lease.identity,handle};
}
/** Terminal cleanup already removed descriptor storage; report only the actual remaining status/control. */
AmaStatus ama_debug_context(const AmaDebugSession *session, AmaDebugContext *output) {
    AmaStatus status = ama_debug_validate(session);
    if (status.code != AMA_OK) { return status; }
    if (output == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    *output = (AmaDebugContext){.context=session->lease.identity,.phase=session->view.control->phase,
        .root=ama_debug_reference(session,session->view.control->root),
        .current=ama_debug_reference(session,session->view.control->current),.status=session->view.status,
        .failed=session->view.world->failed,.type_count=session->view.program->type_count,
        .constant_count=session->view.program->constant_count};
    return ama_status_success();
}
