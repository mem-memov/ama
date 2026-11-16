/**
 * @file
 * @brief Allocate one shared field according to declared ownership and initializer count.
 *
 * [CONST-01](../../../../spec/language/declarations.md#const-01)
 * Reference fields contain no mutable instance identity; partial field arrays remain owned by the enclosing constant.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Allocate the declared field array once with checked count arithmetic. References require no owned child payload; evaluate explicit reference sources without introducing mutable constant handles. Component contract: [CONST-01](../../../../spec/language/declarations.md#const-01). */
AmaStatus ama_constant_field(AmaProgram *program, AmaConstantBuildWork *work) {
    const AmaProgramField *declaration = &work->value->type->fields[work->field];
    AmaFieldKind kind = work->value->type->object->fields[work->field].kind;
    bool list = kind == AMA_FIELD_OWNING_LIST || kind == AMA_FIELD_REFERENCE_LIST;
    bool reference = kind == AMA_FIELD_REFERENCE || kind == AMA_FIELD_REFERENCE_LIST;
    size_t count = work->source != NULL ? work->source->fields[work->field].count : list ? declaration->initializer_count : 1;
    AmaConstantField *field = &work->value->fields[work->field];
    if (count > SIZE_MAX / sizeof(AmaConstantValue)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    if (count != 0 && !reference) {
        field->items = program->world->allocator.allocate(program->world->allocator.context, count * sizeof(AmaConstantValue));
        if (field->items == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
        memset(field->items, 0, count * sizeof(AmaConstantValue));
    }
    field->count = count;
    if (reference && work->source == NULL) {
        for (size_t index = 0; index < declaration->initializer_count; index++) {
            AmaProgramValue selected = {0};
            AmaStatus status = ama_program_evaluate(program, (AmaHandle){0}, work->value, &declaration->initializers[index], false, &selected);
            if (status.code != AMA_OK) { return status; }
            if (selected.handle.generation != 0) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        }
    }
    return ama_status_success();
}

