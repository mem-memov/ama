/** @file
 * @brief Evaluate direct method operands once with the exact list parent grant.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) defaults position via
 * literal zero; [LIST-10](../../../../../spec/language/lists.md#list-10) permits
 * structural source borrowing but never grants authority to an executed body.
 */
#include "internal.h"
#include "runtime/program/values/selection/internal.h"
#include "runtime/program/transfers/internal.h"

/** Select all supplied nonliteral views before preparing any owned parameter. */
static AmaStatus ama_list_insertion_sources(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, AmaListInsertionInputs *inputs) {
    AmaProgramListService *service = method->service;
    AmaProgram *program = service->program;
    if (plan->count > inputs->arity) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    program->world->operation_active = true;
    AmaPoolCell *caller = NULL;
    AmaStatus status = ama_program_cell(program, service->caller, &caller);
    for (size_t index = 0; index < plan->count && status.code == AMA_OK; index++) {
        const AmaProgramInput *input = &plan->inputs[index];
        if (input->field >= inputs->arity || inputs->supplied[input->field] != NULL) {
            status = ama_status_make(AMA_INVALID_STATE, NULL, 0);
            break;
        }
        inputs->supplied[input->field] = input;
        caller->metadata.service.current_operation = input->operation;
        if (input->source.kind != AMA_SOURCE_LITERAL) {
            status = ama_program_evaluate_authorized(program, service->caller, NULL,
                &input->source, false, service, &inputs->values[input->field]);
        }
    }
    program->world->operation_active = false;
    return status;
}

/** Construct the integer parameter once using its selected literal/copy recipe. */
static AmaStatus ama_list_insertion_position(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, const AmaListInsertionInputs *inputs) {
    AmaProgram *program = method->service->program;
    const AmaProgramInput *input = inputs->supplied[0];
    static const unsigned char zero[] = {'0'};
    AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_LITERAL, .literal = {zero, 1}};
    SourceArena arena;
    source_arena_init(&arena, program->world->allocator);
    AmaStatus status = ama_status_success();
    if (input != NULL) {
        if (input->source.kind == AMA_SOURCE_LITERAL) { recipe.literal = input->source.literal; }
        else if (inputs->values[0].type_id != plan->integer_type) { status = ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        else { status = ama_program_value_recipe(program, &arena, &inputs->values[0], input->source.continuation, &recipe); }
    }
    AmaPoolCell *caller = NULL;
    if (status.code == AMA_OK) { status = ama_program_cell(program, method->service->caller, &caller); }
    if (status.code == AMA_OK) {
        caller->metadata.service.current_operation = plan->installations != NULL ? plan->installations[0] : plan->operation;
        status = ama_object_create(program->world, plan->integer_type, &recipe, &method->inputs[0]);
        if (status.code == AMA_OK) { method->owned_inputs |= 1u; }
    }
    source_arena_destroy(&arena);
    return status;
}

AmaStatus ama_list_insertion_inputs(AmaProgramListMethod *method,
    const AmaProgramListInputs *plan, AmaListInsertionInputs *inputs) {
    AmaStatus status = ama_list_insertion_sources(method, plan, inputs);
    if (status.code != AMA_OK) { return status; }
    method->input_count = inputs->arity;
    AmaProgramListService *service = method->service;
    AmaObjectType *owner = ama_world_type(service->program->world, service->owner.type_id);
    uint64_t element_type = owner->fields[service->field].type_id;
    size_t position = inputs->source_position;
    if (inputs->supplied[position] != NULL) {
        if (inputs->supplied[position]->source.kind == AMA_SOURCE_LITERAL ||
            inputs->values[position].form != AMA_FIELD_REFERENCE || inputs->values[position].type_id != element_type) {
            return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
        }
    }
    if (inputs->positioned) {
        status = ama_list_insertion_position(method, plan, inputs);
        if (status.code != AMA_OK) { return status; }
    }
    method->inputs[position] = inputs->supplied[position] != NULL ? inputs->values[position].handle : (AmaHandle){0};
    return ama_status_success();
}
