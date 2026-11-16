/**
 * @file
 * @brief Check native borrow authority and exact nominal type before exposing a stable cell.
 *
 * [CABI-02](../../../../spec/implementation/native-interface.md#cabi-02) forbids
 * treating equal C representations as equal Ama types. [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03)
 * bounds reference resolution to the operation and permitted access.
 */
#include "borrow.h"
#include "runtime/program/calls/call.h"

/** Validate the linear scope and role, then type, publication, lifecycle and access.
 * Every failure precedes output assignment, allowing generated binders to keep
 * partial view storage private until all required children have been resolved.
 * Component contract: [CABI-02](../../../../spec/implementation/native-interface.md#cabi-02), [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03). */
AmaStatus ama_native_borrow_cell(const AmaNativeBorrowScope *scope, AmaHandle handle,
    uint64_t expected_type, AmaNativeBorrowRole role, AmaPoolCell **output) {
    if (scope == NULL || !scope->active || scope->program == NULL || scope->program->world == NULL
        || !scope->program->world->operation_active || scope->program->world->failed
        || !ama_handle_equal(scope->program->current, scope->receiver) || output == NULL
        || expected_type == 0 || (role != AMA_NATIVE_BORROW_READ && role != AMA_NATIVE_BORROW_RECEIVER)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (ama_handle_is_empty(handle)) { return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0); }
    if (handle.type_id != expected_type) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (role == AMA_NATIVE_BORROW_RECEIVER && !ama_handle_equal(handle, scope->receiver)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(scope->program, handle, &cell);
    if (status.code != AMA_OK) { return status; }
    if (role == AMA_NATIVE_BORROW_READ) { status = ama_instance_access(&cell->metadata, AMA_ACCESS_READ); }
    else if (cell->metadata.lifetime.phase != AMA_LIFETIME_LIVE
        || cell->metadata.service.activity != AMA_INSTANCE_RUNNING
        || !cell->metadata.service.return_link.attached
        || !ama_return_link_is_valid(cell->metadata.service.return_link)) {
        status = ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (status.code == AMA_OK) { *output = cell; }
    return status;
}
