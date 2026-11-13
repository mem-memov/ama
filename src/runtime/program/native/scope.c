/**
 * @file
 * @brief Own a native operation gate for one existing running receiver without executing Ama.
 *
 * [CABI-05](../../../../spec/implementation/native-interface.md#cabi-05) forbids
 * escaped views and runtime reentry. Scope end precedes label dispatch and
 * [STACK-03](../../../../spec/implementation/compiler.md#stack-03) cooperative yield.
 */
#include "borrow.h"
#include "runtime/program/calls/call.h"

/** Acquire authority only after exact current identity and Running service validation.
 * The enclosing context supplies exclusive execution; this is not a cross-thread mutex.
 */
AmaStatus ama_native_borrow_begin(AmaProgram *program, AmaHandle receiver, AmaNativeBorrowScope *scope) {
    if (program == NULL || program->world == NULL || scope == NULL || scope->active) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (program->world->operation_active) { return ama_status_make(AMA_BUSY, NULL, 0); }
    if (program->world->failed || !ama_handle_equal(program->current, receiver)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(program, receiver, &cell);
    if (status.code != AMA_OK) { return status; }
    const AmaServiceState *service = &cell->metadata.service;
    if (cell->metadata.lifetime.phase != AMA_LIFETIME_LIVE || service->activity != AMA_INSTANCE_RUNNING
        || !service->return_link.attached || !ama_return_link_is_valid(service->return_link)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *scope = (AmaNativeBorrowScope){.program = program, .receiver = receiver, .active = true};
    program->world->operation_active = true;
    return ama_status_success();
}

/** Release the uniquely owned operation gate and erase the expired borrowed program/receiver. */
AmaStatus ama_native_borrow_end(AmaNativeBorrowScope *scope) {
    if (scope == NULL || !scope->active || scope->program == NULL || scope->program->world == NULL
        || !scope->program->world->operation_active) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    scope->program->world->operation_active = false;
    *scope = (AmaNativeBorrowScope){0};
    return ama_status_success();
}
