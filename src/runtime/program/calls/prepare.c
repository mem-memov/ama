#include <stdint.h>
#include <string.h>
#include "runtime/program/transfers/internal.h"

typedef struct {
    const AmaProgramInput *input;
    AmaProgramValue value;
} AmaPreparedInput;

static AmaStatus ama_call_sources(AmaProgram *program, AmaHandle caller, const AmaProgramInvocation *invocation,
    AmaObjectOperation *protection, AmaPreparedInput *inputs) {
    AmaPoolCell *context = NULL;
    AmaStatus status = ama_program_cell(program, caller, &context);
    if (status.code != AMA_OK) { return status; }
    for (size_t index = 0; index < invocation->input_count; index++) {
        inputs[index].input = &invocation->inputs[index];
        const AmaProgramSource *source = &inputs[index].input->source;
        context->metadata.service.current_operation = inputs[index].input->operation;
        if (source->kind == AMA_SOURCE_LITERAL) { continue; }
        status = ama_program_evaluate(program, caller, NULL, source, false, &inputs[index].value);
        if (status.code != AMA_OK) { return status; }
    }
    /* Select all sources before acquiring any of them: repeated index readers
     * must observe the same pre-reset state and cannot encounter our own lease. */
    for (size_t index = 0; index < invocation->input_count; index++) {
        const AmaProgramValue *value = &inputs[index].value;
        if (inputs[index].input->source.kind == AMA_SOURCE_LITERAL || value->constant != NULL || value->shared_list != NULL) { continue; }
        if (value->form == AMA_FIELD_OWNED) { status = ama_object_preflight(protection, value->handle, true, false); }
        else if (value->form == AMA_FIELD_OWNING_LIST) {
            for (size_t element = 0; element < value->list->count && status.code == AMA_OK; element++) {
                status = ama_object_preflight(protection, value->list->items[element], true, false);
            }
        }
        if (status.code != AMA_OK) { return status; }
    }
    return status;
}

static AmaStatus ama_call_check_fields(AmaProgram *program, AmaHandle target, const AmaProgramType *type,
    const AmaPreparedInput *inputs, size_t input_count) {
    AmaStatus status = ama_status_success();
    AmaObjectOperation check = {.world = program->world};
    source_arena_init(&check.arena, program->world->allocator);
    for (size_t field = 0; field < type->field_count && status.code == AMA_OK; field++) {
        if (type->fields[field].role == AMA_PROGRAM_STATE) { continue; }
        AmaProgramAccess access = {.field = field};
        AmaProgramValue value = {0};
        status = ama_program_select(program, target, &access, false, &value);
        if (status.code != AMA_OK) { break; }
        for (size_t index = 0; index < input_count; index++) {
            if (inputs[index].input->source.kind == AMA_SOURCE_LITERAL) { continue; }
            if (value.list != NULL && inputs[index].value.list == value.list) { status = ama_status_make(AMA_BUSY, NULL, 0); break; }
        }
        if (status.code != AMA_OK) { break; }
        for (size_t index = 0; index < input_count && status.code == AMA_OK; index++) {
            const AmaProgramValue *source = &inputs[index].value;
            if (source->list == NULL || source->container.generation == 0) { continue; }
            size_t count = value.form == AMA_FIELD_OWNED ? 1 : value.form == AMA_FIELD_OWNING_LIST ? value.list->count : 0;
            for (size_t element = 0; element < count && status.code == AMA_OK; element++) {
                AmaHandle root = value.form == AMA_FIELD_OWNED ? value.handle : value.list->items[element];
                bool overlaps = false;
                status = ama_object_contains(program->world, root, source->container, &overlaps);
                if (status.code == AMA_OK && overlaps) { status = ama_status_make(AMA_BUSY, NULL, 0); }
            }
        }
        if (status.code != AMA_OK) { break; }
        if (value.form == AMA_FIELD_OWNED) { status = ama_object_preflight(&check, value.handle, false, false); }
        else if (value.form == AMA_FIELD_OWNING_LIST) {
            for (size_t element = 0; element < value.list->count && status.code == AMA_OK; element++) {
                status = ama_object_preflight(&check, value.list->items[element], false, false);
            }
        }
    }
    ama_object_unprotect(&check);
    source_arena_destroy(&check.arena);
    return status;
}

static AmaStatus ama_call_install(AmaProgram *program, AmaHandle target, size_t field,
    const AmaPreparedInput *input, const AmaObjectProtection *authority) {
    AmaObjectOperation operation = {.world = program->world, .source_authority = authority};
    source_arena_init(&operation.arena, program->world->allocator);
    AmaProgramAccess access = {.field = field};
    AmaProgramValue destination = {0};
    AmaStatus status = ama_program_select(program, target, &access, false, &destination);
    AmaFieldRecipe recipe = {.count = 1, .supplied = true};
    AmaRecipe literal = {0};
    const AmaProgramValue *source = NULL;
    if (status.code == AMA_OK && input == NULL) {
        status = ama_program_field_recipe(program->world, &operation.arena, target, field, &recipe, program);
    } else if (input != NULL && input->input->source.kind == AMA_SOURCE_LITERAL) {
        literal = (AmaRecipe){.constructor = AMA_CONSTRUCT_LITERAL, .literal = input->input->source.literal};
        recipe.children = &literal;
    } else if (input != NULL) { source = &input->value; }
    if (status.code == AMA_OK) { status = ama_program_store(program, &destination, source, &recipe, &operation); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    return status;
}

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
    if (status.code == AMA_OK) { status = ama_call_sources(program, caller, invocation, &protection, inputs); }
    if (status.code == AMA_OK) { status = ama_call_check_fields(program, target, type, inputs, invocation->input_count); }
    for (AmaProgramRole role = AMA_PROGRAM_PARAMETER; role <= AMA_PROGRAM_RESULT && status.code == AMA_OK; role++) {
        for (size_t field = 0; field < type->field_count && status.code == AMA_OK; field++) {
            if (type->fields[field].role != role) { continue; }
            const AmaPreparedInput *input = NULL;
            for (size_t index = 0; index < invocation->input_count; index++) {
                if (invocation->inputs[index].field == field) { input = &inputs[index]; break; }
            }
            if (invocation->installation_operations != NULL) { context->metadata.service.current_operation = invocation->installation_operations[field]; }
            status = ama_call_install(program, target, field, input, protection.protected_first);
        }
    }
    ama_object_unprotect(&protection);
    source_arena_destroy(&protection.arena);
    program->world->operation_active = false;
    if (status.code != AMA_OK && !program->world->failed) { (void)ama_program_release(program, caller); }
    return status;
}
