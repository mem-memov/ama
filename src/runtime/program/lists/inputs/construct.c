/** @file
 * @brief Construct one owned integer parameter with its exact selected recipe.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) defaults positions via
 * literal zero, not native default construction followed by overwrite.
 */
#include "internal.h"
#include "runtime/program/transfers/internal.h"

AmaStatus ama_list_input_integer(AmaProgramListMethod *method, const AmaProgramListInputs *plan,
    const AmaListInputSources *sources, size_t position) {
    AmaProgram *program = method->service->program;
    const AmaProgramInput *input = sources->supplied[position];
    static const unsigned char zero[] = {'0'};
    AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_LITERAL, .literal = {zero, 1}};
    SourceArena arena;
    source_arena_init(&arena, program->world->allocator);
    AmaStatus status = ama_status_success();
    if (input != NULL) {
        if (input->source.kind == AMA_SOURCE_LITERAL) { recipe.literal = input->source.literal; }
        else if (sources->values[position].type_id != plan->integer_type) { status = ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        else { status = ama_program_value_recipe(program, &arena, &sources->values[position], input->source.continuation, &recipe); }
    }
    AmaPoolCell *caller = NULL;
    if (status.code == AMA_OK) { status = ama_program_cell(program, method->service->caller, &caller); }
    if (status.code == AMA_OK) {
        caller->metadata.service.current_operation = plan->installations != NULL ? plan->installations[position] : plan->operation;
        status = ama_object_create(program->world, plan->integer_type, &recipe, &method->inputs[position]);
        if (status.code == AMA_OK) { method->owned_inputs |= 1u << position; }
    }
    source_arena_destroy(&arena);
    return status;
}
