/**
 * @file
 * @brief Evaluate nested indexed selections iteratively in dependency order.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires each index protocol effect in exact resolved order before use.
 */
#include "internal.h"

AmaStatus ama_program_access(AmaProgram *program, AmaHandle current, const AmaConstantValue *shared,
    const AmaProgramAccess *access, bool private_access, AmaProgramValue *output, bool *selection_refusal) {
    if (selection_refusal != NULL) { *selection_refusal = false; }
    size_t depth = 0;
    for (const AmaProgramAccess *node = access; node != NULL; node = node->index) { depth++; }
    size_t selected = 0;
    while (depth != 0) {
        depth--;
        const AmaProgramAccess *node = access;
        for (size_t index = 0; index < depth; index++) { node = node->index; }
        AmaProgramValue value = {0};
        AmaStatus status = node->shared || shared != NULL ? ama_program_shared_base(program, node, shared, &value)
            : ama_program_instance_base(program, current, node, private_access, &value);
        if (status.code == AMA_OK && node->index != NULL) {
            status = ama_program_index(&value, selected);
            if (status.code == AMA_INDEX_BOUNDS && selection_refusal != NULL) { *selection_refusal = true; }
        }
        if (status.code != AMA_OK) { return status; }
        if (depth == 0) { *output = value; return ama_status_success(); }
        const AmaProgramType *type = ama_program_type(program, value.type_id);
        const void *payload;
        status = ama_program_payload(program, &value, &payload);
        if (status.code != AMA_OK) { return status; }
        if (type == NULL || type->index == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        status = type->index(payload, &selected);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}
