#include <stdint.h>
#include <string.h>
#include "internal.h"

AmaStatus ama_program_constant_build(AmaProgram *program, AmaConstantValue *root, uint64_t type_id,
    const AmaProgramSource *initializer, const AmaConstantValue *source) {
    SourceArena arena;
    source_arena_init(&arena, program->world->allocator);
    AmaConstantBuildWork *work = NULL;
    AmaStatus status = ama_constant_begin(program, &arena, root, type_id, initializer, source, NULL, &work);
    while (status.code == AMA_OK) {
        if (work->field < work->value->type->field_count) {
            const AmaObjectField *descriptor = &work->value->type->object->fields[work->field];
            AmaConstantField *field = &work->value->fields[work->field];
            if (work->element == 0 && field->items == NULL && field->count == 0) {
                status = ama_constant_field(program, work);
                if (status.code != AMA_OK) { break; }
            }
            if (descriptor->kind == AMA_FIELD_REFERENCE || descriptor->kind == AMA_FIELD_REFERENCE_LIST || work->element == field->count) {
                work->field++;
                work->element = 0;
                continue;
            }
            const AmaProgramField *declaration = &work->value->type->fields[work->field];
            const AmaProgramSource *child_initializer = work->source != NULL || declaration->initializer_count == 0
                ? NULL : &declaration->initializers[work->element];
            const AmaConstantValue *child_source = work->source == NULL ? NULL : &work->source->fields[work->field].items[work->element];
            AmaConstantBuildWork *child = NULL;
            status = ama_constant_begin(program, &arena, &field->items[work->element], descriptor->type_id,
                child_initializer, child_source, work, &child);
            if (child != NULL) { work = child; }
        } else {
            status = ama_constant_payload(program, work);
            if (status.code != AMA_OK || work->parent == NULL) { break; }
            work = work->parent;
            work->element++;
        }
    }
    if (status.code != AMA_OK) {
        if (program->world->report != NULL) { program->world->report(program->world->report_context, status); }
        status = ama_program_constant_cleanup(program, root, status);
    }
    source_arena_destroy(&arena);
    return status;
}

AmaStatus ama_program_constants_initialize(AmaProgram *program) {
    for (size_t index = 0; index < program->constant_count; index++) {
        const AmaProgramConstant *declaration = &program->constants[index];
        program->initialization_operation = declaration->operation;
        program->world->construction_operation = declaration->operation;
        size_t count = declaration->list ? declaration->initializer_count : 1;
        if (count > SIZE_MAX / sizeof(AmaConstantValue)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
        if (count != 0) {
            declaration->storage->items = program->world->allocator.allocate(program->world->allocator.context, count * sizeof(AmaConstantValue));
            if (declaration->storage->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
            memset(declaration->storage->items, 0, count * sizeof(AmaConstantValue));
        }
        declaration->storage->count = count;
        for (size_t element = 0; element < count; element++) {
            const AmaProgramSource *initializer = declaration->initializer_count == 0 ? NULL : &declaration->initializers[element];
            AmaStatus status = ama_program_constant_build(program, &declaration->storage->items[element], declaration->type_id, initializer, NULL);
            if (status.code != AMA_OK) { return status; }
        }
    }
    return ama_status_success();
}
