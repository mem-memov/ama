/** @file
 * @brief Resolve checked control schemas without accepting foreign-context aliases.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) stays within
 * one context; [REF-02](../../../../../../spec/language/values-and-references.md#ref-02)
 * forbids equating local triples from different mutable worlds.
 */
#include "internal.h"

AmaStatus ama_foreach_copy_type(void *context, AmaWorld *world, uint64_t identity, const AmaProgramType **output) {
    const AmaProgram *program = context;
    if (program == NULL || program->world != world) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    const AmaProgramType *type = ama_program_type(program, identity);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    *output = type;
    return ama_status_success();
}

AmaStatus ama_foreach_copy_list(const AmaProgramType *type, const void *storage,
    size_t field, const AmaHandleList **output) {
    if (field >= type->object->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaObjectField *descriptor = &type->object->fields[field];
    if (descriptor->kind != AMA_FIELD_OWNING_LIST && descriptor->kind != AMA_FIELD_REFERENCE_LIST) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *output = (const void *)((const unsigned char *)storage + descriptor->offset);
    return ama_status_success();
}
