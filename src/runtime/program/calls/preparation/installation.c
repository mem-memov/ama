/**
 * @file
 * @brief Install one selected constructor per field using borrowed source authority.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires parameters then results in declaration order, with per-field commit.
 */
#include "internal.h"

/** Select literal/value/default exactly once for this field. All per-field
 * scratch and borrowed-authority protections end before returning; store owns
 * replacement's initialized-prefix cleanup and preserves its primary failure.
 * Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
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
    } else if (input != NULL && input->input->source.kind == AMA_SOURCE_PREPARED) {
        if (input->input->source.prepared == NULL || input->input->source.type_id != destination.type_id) {
            status = ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
        } else { recipe = *input->input->source.prepared; }
    } else if (input != NULL) { source = &input->value; }
    if (status.code == AMA_OK) { status = ama_program_store(program, &destination, source, &recipe, &operation); }
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    return status;
}

/** Install parameters before results, each in declaration order. Earlier commits
 * remain on later failure; defaults read earlier new values. source authority is
 * borrowed from the outer protection context and never released here.
 * Component contract: [CALL-08](../../../../../spec/language/calls-and-results.md#call-08). */
AmaStatus ama_call_install_fields(AmaProgram *program, AmaHandle target, const AmaProgramType *type,
    const AmaProgramInvocation *invocation, const AmaPreparedInput *inputs,
    const AmaObjectProtection *authority, uint64_t *current_operation) {
    AmaStatus status = ama_status_success();
    for (AmaProgramRole role = AMA_PROGRAM_PARAMETER; role <= AMA_PROGRAM_RESULT && status.code == AMA_OK; role++) {
        for (size_t field = 0; field < type->field_count && status.code == AMA_OK; field++) {
            if (type->fields[field].role != role) { continue; }
            const AmaPreparedInput *input = NULL;
            for (size_t index = 0; index < invocation->input_count; index++) {
                if (invocation->inputs[index].field == field) { input = &inputs[index]; break; }
            }
            if (invocation->installation_operations != NULL) { *current_operation = invocation->installation_operations[field]; }
            status = ama_call_install(program, target, field, input, authority);
        }
    }
    return status;
}
