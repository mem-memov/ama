/**
 * @file
 * @brief Select receivers before sources and install typed values under one operation guard.
 * [LIST-13](../../../../spec/language/lists.md#list-13) requires exact occurrence retirement and replacement namespaces.
 *
 * [LIST-10](../../../../spec/language/lists.md#list-10) grants busy-list receiver access only to a method-direct structural transfer.
 * [REF-03](../../../../spec/language/values-and-references.md#ref-03) keeps
 * reference assignments as handle copies without acquiring their targets.
 * [CONT-04](../../../../spec/language/continuations.md#cont-04) preserves positions
 * only when the selected source explicitly carries continuation-copy authority.
 * Owned replacement stages construction before destructive commit.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) Only a different explicit handle assignment detaches a binding origin; plain reference copies acquire no association.
 */
#include "internal.h"
#include "runtime/program/values/selection/internal.h"
#include "runtime/program/lists/traversal/traversal.h"

/** Require an owned scalar source; produce an immutable-constant recipe or exact handle clone recipe with explicit continuation mode. Arena/source lifetimes cover construction. Component contract: [LIST-13](../../../../spec/language/lists.md#list-13), [LIST-10](../../../../spec/language/lists.md#list-10), [REF-03](../../../../spec/language/values-and-references.md#ref-03), [CONT-04](../../../../spec/language/continuations.md#cont-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_value_recipe(AmaProgram *program, SourceArena *arena, const AmaProgramValue *value,
    bool continuation, AmaRecipe *recipe) {
    if (value->form != AMA_FIELD_OWNED) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (value->constant != NULL) { return ama_program_constant_recipe(program, arena, value->constant, recipe); }
    *recipe = (AmaRecipe){.constructor = AMA_CONSTRUCT_CLONE, .source = value->handle, .continuation = continuation};
    return ama_status_success();
}

/** Install one typed scalar or delegate whole-list transfer. Reference assignment copies handles without target acquisition; owning values use staged reconstruction in the caller operation. Component contract: [LIST-13](../../../../spec/language/lists.md#list-13), [LIST-10](../../../../spec/language/lists.md#list-10), [REF-03](../../../../spec/language/values-and-references.md#ref-03), [CONT-04](../../../../spec/language/continuations.md#cont-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
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
        AmaHandle target = source != NULL ? source->handle
            : initializers != NULL && initializers->references != NULL ? initializers->references[0] : (AmaHandle){0};
        if (destination->membership_list != NULL) {
            const AmaMembershipRecord *record = NULL;
            AmaStatus status = ama_membership_lookup(destination->membership_list, destination->membership, &record);
            if (status.code != AMA_OK) { return status; }
            AmaMembership identity;
            return ama_membership_replace(destination->membership_list, record->position, target, &identity);
        }
        ama_program_foreach_detach(destination, target);
        *destination->location = target;
        return ama_status_success();
    }
    AmaRecipe recipe = {0};
    AmaStatus status = ama_status_success();
    if (source != NULL) { status = ama_program_value_recipe(program, &operation->arena, source, source->continuation, &recipe); }
    else if (initializers != NULL && initializers->children != NULL) { recipe = initializers->children[0]; }
    return status.code == AMA_OK ? ama_object_reconstruct_in(operation, destination->handle, &recipe) : status;
}

/** Own guard, arena and protections for selection plus store of an already selected source; release temporary authority on every return. Component contract: [LIST-13](../../../../spec/language/lists.md#list-13), [LIST-10](../../../../spec/language/lists.md#list-10), [REF-03](../../../../spec/language/values-and-references.md#ref-03), [CONT-04](../../../../spec/language/continuations.md#cont-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_transfer_authorized(AmaProgram *program, AmaHandle current, const AmaProgramAccess *receiver,
    const AmaProgramValue *source, const struct AmaProgramListService *authority) {
    if (program->world->operation_active || program->world->failed) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaObjectOperation operation = {.world = program->world};
    source_arena_init(&operation.arena, program->world->allocator);
    AmaProgramValue destination = {0};
    AmaStatus status = ama_program_access_authorized(program, current, current, NULL,
        receiver, false, authority, &destination, NULL);
    if (status.code == AMA_OK) { status = ama_program_store(program, &destination, source, NULL, &operation); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    program->world->operation_active = false;
    return status;
}

/** Executed module bodies cannot inherit a list grant through ordinary transfers. Component contract: [LIST-13](../../../../spec/language/lists.md#list-13), [LIST-10](../../../../spec/language/lists.md#list-10), [REF-03](../../../../spec/language/values-and-references.md#ref-03), [CONT-04](../../../../spec/language/continuations.md#cont-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
AmaStatus ama_program_transfer(AmaProgram *program, AmaHandle current, const AmaProgramAccess *receiver,
    const AmaProgramValue *source) {
    return ama_program_transfer_authorized(program, current, receiver, source, NULL);
}

/** Select receiver first and source once, then install under one guard. Explicit owned continuation copy uses continuation replacement; release protections/scratch on every path. Component contract: [LIST-13](../../../../spec/language/lists.md#list-13), [LIST-10](../../../../spec/language/lists.md#list-10), [REF-03](../../../../spec/language/values-and-references.md#ref-03), [CONT-04](../../../../spec/language/continuations.md#cont-04), [FLOW-05](../../../../spec/language/lists.md#flow-05). */
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
