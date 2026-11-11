#include <stdint.h>
#include <string.h>
#include "internal.h"

AmaStatus ama_constant_begin(AmaProgram *program, SourceArena *arena, AmaConstantValue *value,
    uint64_t type_id, const AmaProgramSource *initializer, const AmaConstantValue *source,
    AmaConstantBuildWork *parent, AmaConstantBuildWork **output) {
    uint64_t operation = parent == NULL ? program->initialization_operation : parent->source != NULL ? parent->operation
        : parent->value->type->fields[parent->field].operation;
    program->world->construction_operation = operation;
    AmaConstantBuildWork *work = source_arena_allocate(arena, sizeof(*work));
    if (work == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    if (initializer != NULL && initializer->kind != AMA_SOURCE_LITERAL) {
        AmaProgramValue selected = {0};
        AmaStatus status = ama_program_evaluate(program, (AmaHandle){0}, parent == NULL ? NULL : parent->value, initializer, false, &selected);
        if (status.code != AMA_OK) { return status; }
        if (selected.constant == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        source = selected.constant;
    }
    const AmaProgramType *type = ama_program_type(program, type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (source != NULL && source->type != type) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    *work = (AmaConstantBuildWork){.parent = parent, .value = value, .source = source, .initializer = initializer,
        .operation = operation};
    *value = (AmaConstantValue){.type = type, .parent = parent == NULL ? NULL : parent->value, .remaining_fields = type->field_count};
    if (type->field_count > SIZE_MAX / sizeof(AmaConstantField)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    if (type->field_count != 0) {
        value->fields = program->world->allocator.allocate(program->world->allocator.context, type->field_count * sizeof(AmaConstantField));
        if (value->fields == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
        memset(value->fields, 0, type->field_count * sizeof(AmaConstantField));
    }
    *output = work;
    return ama_status_success();
}

