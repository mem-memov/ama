/**
 * @file
 * @brief Select immutable shared field storage without materializing pool instances.
 *
 * [CONST-01](../../../../../spec/language/declarations.md#const-01)
 * keeps shared storage separate from mutable instance identities.
 */
#include "internal.h"

AmaStatus ama_program_shared_base(AmaProgram *program, const AmaProgramAccess *access,
    const AmaConstantValue *current, AmaProgramValue *value) {
    const AmaConstantField *field;
    if (access->shared) {
        if (access->field >= program->constant_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        const AmaProgramConstant *declaration = &program->constants[access->field];
        field = declaration->storage;
        value->type_id = declaration->type_id;
        value->form = declaration->list ? AMA_FIELD_OWNING_LIST : AMA_FIELD_OWNED;
    } else {
        if (current == NULL || access->field >= current->type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        const AmaObjectField *descriptor = &current->type->object->fields[access->field];
        field = &current->fields[access->field];
        value->type_id = descriptor->type_id;
        value->form = descriptor->kind;
    }
    if (value->form == AMA_FIELD_OWNING_LIST || value->form == AMA_FIELD_REFERENCE_LIST) {
        value->shared_list = field;
    } else if (value->form == AMA_FIELD_OWNED) {
        if (field->count != 1 || field->items == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        value->constant = field->items;
    }
    return ama_status_success();
}
