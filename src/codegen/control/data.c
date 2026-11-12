/**
 * @file
 * @brief Traverse resolved control nodes and compose their operation-specific data.
 *
 * [ARCH-02](../../../spec/implementation/compiler.md#arch-02) separates model
 * traversal from descriptor families; [ARCH-05](../../../spec/implementation/compiler.md#arch-05)
 * requires explicit IR operations rather than guessed source behavior.
 */
#include "codegen/internal.h"
#include "data/data.h"

/** Allocate a stable operation identifier in the model arena; NULL records failure. */
const char *codegen_control_name(NativeScaffold *scaffold, const ScaffoldType *type, const ControlNode *node) {
    return codegen_indexed_name(scaffold, type->names[SCAFFOLD_WRITE]->internal_name, "Operation", node->selector);
}

/** Compose descriptors for the finite validated model; abort before publication
 * if an operation-specific child cannot resolve required IR or write bytes.
 */
bool codegen_emit_controls(NativeScaffold *scaffold, FILE *output) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            const char *name = codegen_control_name(scaffold, type, node);
            if (name == NULL) { return false; }
            if (node->operation == CONTROL_ACQUIRE_INSTANCE) {
                if (!codegen_control_invocation(scaffold, output, type, node, name)) { return false; }
            } else if (node->operation == CONTROL_SELECT_RECEIVER || node->operation == CONTROL_CONDITION || node->operation == CONTROL_TRANSFER_OUTPUT) {
                const SemanticAccess *access = node->operation == CONTROL_TRANSFER_OUTPUT ? node->binding->receiver : node->statement->access;
                if (!codegen_access_definition(scaffold, output, name, access)) { return false; }
                if (node->operation == CONTROL_SELECT_RECEIVER
                    && !codegen_control_assignment_data(scaffold, output, node, name)) { return false; }
                if (node->operation == CONTROL_TRANSFER_OUTPUT
                    && !codegen_control_output_data(scaffold, output, node->binding->field, name)) { return false; }
            }
        }
    }
    return true;
}
