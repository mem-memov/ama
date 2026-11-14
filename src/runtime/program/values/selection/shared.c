/**
 * @file
 * @brief Select immutable shared field storage without materializing pool instances.
 *
 * [CONST-01](../../../../../spec/language/declarations.md#const-01)
 * keeps shared storage separate from mutable instance identities.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) gives immutable
 * selections no mutable binding origin, even when reusing a temporary view.
 */
#include "internal.h"

AmaStatus ama_program_shared_base(AmaProgram *program, const AmaProgramAccess *access,
    const AmaConstantValue *current, AmaProgramValue *value) {
    AmaProgramValue selected = {0};
    const AmaConstantField *field;
    if (access->shared) {
        if (access->field >= program->constant_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        const AmaProgramConstant *declaration = &program->constants[access->field];
        field = declaration->storage;
        selected.type_id = declaration->type_id;
        selected.form = declaration->list ? AMA_FIELD_OWNING_LIST : AMA_FIELD_OWNED;
    } else {
        if (current == NULL || access->field >= current->type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        const AmaObjectField *descriptor = &current->type->object->fields[access->field];
        field = &current->fields[access->field];
        selected.type_id = descriptor->type_id;
        selected.form = descriptor->kind;
    }
    if (selected.form == AMA_FIELD_OWNING_LIST || selected.form == AMA_FIELD_REFERENCE_LIST) {
        selected.shared_list = field;
    } else if (selected.form == AMA_FIELD_OWNED) {
        if (field->count != 1 || field->items == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        selected.constant = field->items;
    }
    *value = selected;
    return ama_status_success();
}
