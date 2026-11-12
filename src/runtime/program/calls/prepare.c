/**
 * @file
 * @brief Own entry preparation guard, source leases and phase ordering.
 *
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08) requires source selection,
 * protection and affected-field checks before declaration-order installation.
 * Failure releases temporary leases and the acquired call unless cleanup is terminal.
 */
#include <stdint.h>
#include <string.h>
#include "preparation/internal.h"

/** Prepare only entry calls; read/resume modes preserve interface state. The
 * invocation is borrowed, scratch/source leases end before any Ama body begins,
 * and earlier field commits survive failure of a later constructor.
 */
AmaStatus ama_program_prepare(AmaProgram *program, AmaHandle caller, const AmaProgramInvocation *invocation) {
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaPoolCell *context = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK) { return status; }
    AmaHandle target = context->metadata.service.outgoing;
    const AmaProgramType *type = ama_program_type(program, target.type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (invocation->mode != AMA_ACCESS_ENTRY) { return ama_status_success(); }
    program->world->operation_active = true;
    AmaObjectOperation protection = {.world = program->world};
    source_arena_init(&protection.arena, program->world->allocator);
    AmaPreparedInput *inputs = NULL;
    if (invocation->input_count > SIZE_MAX / sizeof(*inputs)) { status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    else if (invocation->input_count != 0) {
        inputs = source_arena_allocate(&protection.arena, invocation->input_count * sizeof(*inputs));
        if (inputs == NULL) { status = ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
        else { memset(inputs, 0, invocation->input_count * sizeof(*inputs)); }
    }
    if (status.code == AMA_OK) { status = ama_call_select_sources(program, caller, invocation, inputs); }
    if (status.code == AMA_OK) { status = ama_call_protect_sources(&protection, inputs, invocation->input_count); }
    if (status.code == AMA_OK) { status = ama_call_check_fields(program, target, type, inputs, invocation->input_count); }
    if (status.code == AMA_OK) { status = ama_call_install_fields(program, target, type, invocation,
        inputs, protection.protected_first, &context->metadata.service.current_operation); }
    ama_object_unprotect(&protection);
    source_arena_destroy(&protection.arena);
    program->world->operation_active = false;
    if (status.code != AMA_OK && !program->world->failed) { (void)ama_program_release(program, caller); }
    return status;
}
