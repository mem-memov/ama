/** @file
 * @brief Stream only currently live actual instances while holding one exclusive inspection session.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves reclaimed directory holes;
 * [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) excludes freed history.
 */
#include "../internal.h"
/** Synchronous wrapper owns no instance or native payload and retains a first inspection error only. Component contract: [RT-02](../../../../spec/implementation/runtime.md#rt-02), [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
typedef struct {
    AmaDebugSession *session;
    AmaDebugInstanceVisitor visit;
    void *context;
    AmaStatus status;
    bool stopped;
} DebugInstanceVisit;
/** Public slot identity is wrapped with the supplying context before the ordinary snapshot helper validates it. Component contract: [RT-02](../../../../spec/implementation/runtime.md#rt-02), [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
static bool debug_instance_visit(void *context, AmaHandle handle, const AmaPoolCell *cell) {
    (void)cell;
    DebugInstanceVisit *visitor = context;
    AmaDebugInstance snapshot;
    visitor->status = ama_debug_instance(visitor->session,ama_debug_reference(visitor->session,handle),&snapshot);
    if (visitor->status.code != AMA_OK) { return false; }
    visitor->stopped = !visitor->visit(visitor->context,&snapshot);
    return !visitor->stopped;
}
/** Metadata enumeration leaves liveness, acquisition and pool layout unchanged; callbacks may make nested read queries. Component contract: [RT-02](../../../../spec/implementation/runtime.md#rt-02), [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
AmaStatus ama_debug_instances(AmaDebugSession *session, AmaDebugInstanceVisitor visit, void *context) {
    if (visit == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    AmaStatus status = ama_debug_visit_begin(session);
    if (status.code != AMA_OK) { return status; }
    DebugInstanceVisit visitor = {.session=session,.visit=visit,.context=context,.status=ama_status_success()};
    for (size_t index = 0; index < session->view.program->type_count; index++) {
        const AmaProgramType *type = &session->view.program->types[index];
        if (type->object == NULL || type->object->pool.type_id == 0) { continue; }
        status = ama_pool_visit(&type->object->pool,debug_instance_visit,&visitor);
        if (status.code != AMA_OK || visitor.status.code != AMA_OK || visitor.stopped) { break; }
    }
    ama_debug_visit_end(session);
    return status.code == AMA_OK ? visitor.status : status;
}
