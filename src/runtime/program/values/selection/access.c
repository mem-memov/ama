/**
 * @file
 * @brief Evaluate nested indexed selections iteratively in dependency order.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires each index protocol effect in exact resolved order before use.
 * [ERROR-05](../../../../../spec/implementation/diagnostics.md#error-05)
 * makes only a valid dedicated invalid-index result a conversion refusal;
 * malformed transport and other decoder failures never consume the output.
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09) separates
 * the selected result owner's base from caller-local nested index operands.
 */
#include "internal.h"

/** Resolve the selected base in its protected owner while evaluating every index
 * operand in the initiating caller, as required by
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09).
 * Ordinary selection passes the same identity for both domains.
 */
AmaStatus ama_program_access_authorized(AmaProgram *program, AmaHandle current, AmaHandle owner, const AmaConstantValue *shared,
    const AmaProgramAccess *access, bool private_access, const struct AmaProgramListService *authority,
    AmaProgramValue *output, bool *selection_refusal) {
    if (selection_refusal != NULL) { *selection_refusal = false; }
    if (authority != NULL && !ama_handle_equal(current, program->current)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    size_t depth = 0;
    for (const AmaProgramAccess *node = access; node != NULL; node = node->index) { depth++; }
    size_t selected = 0;
    while (depth != 0) {
        depth--;
        const AmaProgramAccess *node = access;
        for (size_t index = 0; index < depth; index++) { node = node->index; }
        AmaProgramValue value = {0};
        bool list_refusal = false;
        AmaStatus status = node->shared || shared != NULL ? ama_program_shared_base(program, node, shared, &value)
            : ama_program_instance_authorized(program, depth == 0 ? owner : current, node,
                private_access, authority, &list_refusal, &value);
        if (list_refusal && selection_refusal != NULL) { *selection_refusal = true; }
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
        status = ama_status_checked(type->index(payload, &selected));
        if (status.code == AMA_INVALID_INDEX && selection_refusal != NULL) {
            *selection_refusal = true;
        }
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

AmaStatus ama_program_access_from(AmaProgram *program, AmaHandle current, AmaHandle owner,
    const AmaConstantValue *shared, const AmaProgramAccess *access, bool private_access,
    AmaProgramValue *output, bool *selection_refusal) {
    return ama_program_access_authorized(program, current, owner, shared, access, private_access, NULL, output, selection_refusal);
}

AmaStatus ama_program_access(AmaProgram *program, AmaHandle current, const AmaConstantValue *shared,
    const AmaProgramAccess *access, bool private_access, AmaProgramValue *output, bool *selection_refusal) {
    return ama_program_access_from(program, current, current, shared, access, private_access, output, selection_refusal);
}
