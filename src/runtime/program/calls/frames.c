/** @file
 * @brief Borrow fixed caller operation slots and restore enclosing acquisitions.
 * [LIST-04](../../../../spec/language/lists.md#list-04) permits a root intrinsic value to park an empty outgoing parent.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) bounds these
 * transient slots by syntax, excludes them from copies, and releases selected
 * children before restoring their still-acquired parent. No C activation survives
 * an Ama transfer, and no application payload is saved or restored.
 */
#include "call.h"

AmaStatus ama_program_frame(AmaProgram *program, AmaHandle caller, size_t offset, AmaProgramFrame **output) {
    AmaObjectType *type = ama_world_type(program->world, caller.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(program, caller, &cell);
    if (status.code != AMA_OK) { return status; }
    if (offset % _Alignof(AmaProgramFrame) != 0 || offset > type->pool.payload_size
        || sizeof(AmaProgramFrame) > type->pool.payload_size - offset) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *output = (AmaProgramFrame *)((unsigned char *)ama_pool_payload(&type->pool, cell) + offset);
    return ama_status_success();
}

AmaStatus ama_program_release_result(AmaProgram *program, AmaHandle caller, size_t offset, bool abandoning) {
    AmaProgramFrame *frame = NULL;
    AmaStatus status = ama_program_frame(program, caller, offset, &frame);
    if (status.code != AMA_OK) { return status; }
    if (!frame->active) { return abandoning ? ama_status_success() : ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaPoolCell *context = NULL, *parent = NULL;
    status = ama_program_cell(program, caller, &context);
    if (status.code == AMA_OK && !ama_handle_is_empty(frame->parent)) {
        status = ama_program_cell(program, frame->parent, &parent);
    }
    if (status.code != AMA_OK) { return status; }
    if (parent != NULL && parent->metadata.service.activity != AMA_INSTANCE_ACQUIRED) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    status = ama_program_release(program, caller);
    if (status.code != AMA_OK) { return status; }
    context->metadata.service.outgoing = frame->parent;
    context->metadata.service.outgoing_previous = frame->previous;
    *frame = (AmaProgramFrame){0};
    return ama_status_success();
}
