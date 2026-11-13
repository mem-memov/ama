/**
 * @file
 * @brief Select receivers before sources and install typed values under one operation guard.
 *
 * [REF-03](../../../../spec/language/values-and-references.md#ref-03)
 * Reference assignment copies handles without acquiring targets; owned replacement stages construction and explicit condition protocols may fail technically.
 * [PROTO-04](../../../../spec/implementation/native-interface.md#proto-04)
 * reads current state; checked condition failure never publishes a callback output.
 */
#include "internal.h"

/** Require an owned scalar source; produce an immutable-constant recipe or exact handle clone recipe with explicit continuation mode. Arena/source lifetimes cover construction. */
AmaStatus ama_program_value_recipe(AmaProgram *program, SourceArena *arena, const AmaProgramValue *value,
    bool continuation, AmaRecipe *recipe) {
    if (value->form != AMA_FIELD_OWNED) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (value->constant != NULL) { return ama_program_constant_recipe(program, arena, value->constant, recipe); }
    *recipe = (AmaRecipe){.constructor = AMA_CONSTRUCT_CLONE, .source = value->handle, .continuation = continuation};
    return ama_status_success();
}

/** Install one typed scalar or delegate whole-list transfer. Reference assignment copies handles without target acquisition; owning values use staged reconstruction in the caller operation. */
AmaStatus ama_program_store(AmaProgram *program, AmaProgramValue *destination, const AmaProgramValue *source,
    const AmaFieldRecipe *initializers, AmaObjectOperation *operation) {
    if (destination->form == AMA_FIELD_OWNING_LIST || destination->form == AMA_FIELD_REFERENCE_LIST) {
        return ama_program_store_list(program, destination, source, initializers, operation);
    }
    if (destination->location == NULL || (source != NULL && source->type_id != destination->type_id)) {
        return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
    }
    if (destination->form == AMA_FIELD_REFERENCE) {
        if (source != NULL && source->form != AMA_FIELD_REFERENCE) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        *destination->location = source != NULL ? source->handle
            : initializers != NULL && initializers->references != NULL ? initializers->references[0] : (AmaHandle){0};
        return ama_status_success();
    }
    AmaRecipe recipe = {0};
    AmaStatus status = ama_status_success();
    if (source != NULL) { status = ama_program_value_recipe(program, &operation->arena, source, false, &recipe); }
    else if (initializers != NULL && initializers->children != NULL) { recipe = initializers->children[0]; }
    return status.code == AMA_OK ? ama_object_reconstruct_in(operation, destination->handle, &recipe) : status;
}

/** Own guard, arena and protections for selection plus store of an already selected source; release temporary authority on every return. */
AmaStatus ama_program_transfer(AmaProgram *program, AmaHandle current, const AmaProgramAccess *receiver,
    const AmaProgramValue *source) {
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaObjectOperation operation = {.world = program->world};
    source_arena_init(&operation.arena, program->world->allocator);
    AmaProgramValue destination = {0};
    AmaStatus status = ama_program_select(program, current, receiver, false, &destination);
    if (status.code == AMA_OK) { status = ama_program_store(program, &destination, source, NULL, &operation); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    program->world->operation_active = false;
    return status;
}

/** Select receiver first and source once, then install under one guard. Explicit owned continuation copy uses continuation replacement; release protections/scratch on every path. */
AmaStatus ama_program_assign(AmaProgram *program, AmaHandle current, const AmaProgramAccess *receiver,
    const AmaProgramSource *source) {
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaProgramValue destination = {0}, value = {0};
    AmaStatus status = ama_program_select(program, current, receiver, false, &destination);
    if (status.code == AMA_OK) { status = ama_program_evaluate(program, current, NULL, source, false, &value); }
    if (status.code != AMA_OK) { program->world->operation_active = false; return status; }
    AmaObjectOperation operation = {.world = program->world};
    source_arena_init(&operation.arena, program->world->allocator);
    if (source->continuation && destination.form == AMA_FIELD_OWNED) {
        status = ama_object_replace_in(&operation, destination.handle, value.handle, true);
    } else { status = ama_program_store(program, &destination, &value, NULL, &operation); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    program->world->operation_active = false;
    return status;
}

/** Select a readable scalar and call its condition protocol under one guard.
 * Check transport before publishing the local output. Every failure preserves
 * caller output and releases the guard; no numeric code becomes implicit truth.
 */
AmaStatus ama_program_condition(AmaProgram *program, AmaHandle current, const AmaProgramAccess *access, bool *result) {
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaProgramValue value = {0};
    AmaStatus status = ama_program_select(program, current, access, false, &value);
    const void *payload = NULL;
    if (status.code == AMA_OK) { status = ama_program_payload(program, &value, &payload); }
    const AmaProgramType *type = ama_program_type(program, value.type_id);
    if (status.code == AMA_OK) {
        bool condition = false;
        status = type == NULL || type->condition == NULL ? ama_status_make(AMA_TYPE_MISMATCH, NULL, 0)
            : ama_status_checked(type->condition(payload, &condition));
        if (status.code == AMA_OK) { *result = condition; }
    }
    program->world->operation_active = false;
    return status;
}
