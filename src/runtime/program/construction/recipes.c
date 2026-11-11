#include <stdint.h>
#include <string.h>
#include "runtime/program/internal.h"

static void *ama_program_recipe_array(SourceArena *arena, size_t count, size_t width) {
    if (count == 0 || count > SIZE_MAX / width) { return NULL; }
    void *items = source_arena_allocate(arena, count * width);
    if (items != NULL) { memset(items, 0, count * width); }
    return items;
}

AmaStatus ama_program_resolve_recipes(AmaProgram *program, SourceArena *arena, AmaHandle current,
    const AmaConstantValue *shared, const AmaProgramSource *sources, size_t source_count,
    AmaFieldKind kind, AmaFieldRecipe *output) {
    bool list = kind == AMA_FIELD_OWNING_LIST || kind == AMA_FIELD_REFERENCE_LIST;
    bool reference = kind == AMA_FIELD_REFERENCE || kind == AMA_FIELD_REFERENCE_LIST;
    size_t count = list ? source_count : 1;
    *output = (AmaFieldRecipe){.count = count, .supplied = true};
    if (source_count == 0) { return ama_status_success(); }
    AmaRecipe *children = reference ? NULL : ama_program_recipe_array(arena, count, sizeof(AmaRecipe));
    AmaHandle *references = reference ? ama_program_recipe_array(arena, count, sizeof(AmaHandle)) : NULL;
    if ((reference ? (void *)references : (void *)children) == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    for (size_t index = 0; index < count; index++) {
        const AmaProgramSource *source = &sources[index];
        if (source->kind == AMA_SOURCE_LITERAL) {
            if (reference) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
            children[index] = (AmaRecipe){.constructor = AMA_CONSTRUCT_LITERAL, .literal = source->literal};
            continue;
        }
        AmaProgramValue value = {0};
        AmaStatus status = ama_program_evaluate(program, current, shared, source, true, &value);
        if (status.code != AMA_OK) { return status; }
        if (reference) {
            if (value.form != AMA_FIELD_REFERENCE) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
            references[index] = value.handle;
        } else if (value.constant != NULL) {
            status = ama_program_constant_recipe(program, arena, value.constant, &children[index]);
            if (status.code != AMA_OK) { return status; }
        } else {
            if (value.form != AMA_FIELD_OWNED) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
            children[index] = (AmaRecipe){.constructor = AMA_CONSTRUCT_CLONE,
                .source = value.handle, .continuation = source->continuation};
        }
    }
    output->children = children;
    output->references = references;
    return ama_status_success();
}

AmaStatus ama_program_field_recipe(AmaWorld *world, SourceArena *arena, AmaHandle parent,
    size_t field, AmaFieldRecipe *output, void *context) {
    (void)world;
    AmaProgram *program = context;
    const AmaProgramType *type = ama_program_type(program, parent.type_id);
    if (type == NULL || field >= type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaProgramField *declaration = &type->fields[field];
    if (program->current.generation == 0) {
        program->initialization_operation = declaration->operation;
        program->world->construction_operation = declaration->operation;
    }
    AmaStatus status = ama_program_resolve_recipes(program, arena, parent, NULL,
        declaration->initializers, declaration->initializer_count, type->object->fields[field].kind, output);
    if (status.code == AMA_OK) { output->operation = declaration->operation; }
    return status;
}
