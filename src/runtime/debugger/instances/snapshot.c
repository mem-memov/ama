/** @file
 * @brief Project service/lifecycle state and exact recorded source mappings without reading native State.
 * [DEBUG-01](../../../../spec/implementation/diagnostics.md#debug-01) maps exact selectors;
 * [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) keeps caller links transient.
 */
#include "../internal.h"
/** Recorded operation and saved resume are distinct observations; no historical caller is reconstructed. Component contract: [DEBUG-01](../../../../spec/implementation/diagnostics.md#debug-01), [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
AmaStatus ama_debug_instance(const AmaDebugSession *session, AmaContextReference reference, AmaDebugInstance *output) {
    if (output == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    const AmaProgramType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_debug_resolve(session,reference,&type,&cell);
    if (status.code != AMA_OK) { return status; }
    const AmaDebugType *name = ama_debug_type(session,reference.local.type_id);
    const AmaServiceState *service = &cell->metadata.service;
    *output = (AmaDebugInstance){.reference=reference,.owner=ama_debug_reference(session,cell->owner),
        .type_name=name == NULL ? NULL : name->name,.activity=service->activity,.lifetime=cell->metadata.lifetime.phase,
        .resume=service->resume,.caller=service->return_link,.outgoing=ama_debug_reference(session,service->outgoing),
        .recorded_operation=service->current_operation,.recorded_location=ama_debug_location(session,service->current_operation),
        .resume_location=service->resume.valid ? ama_debug_location(session,service->resume.selector) : NULL,
        .last_refusal=service->last_refusal,.native_opaque=type->object->hooks.initialize != NULL};
    return ama_status_success();
}
