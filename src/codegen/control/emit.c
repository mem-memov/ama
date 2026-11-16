/**
 * @file
 * @brief Dispatch resolved control nodes to narrow C writers and the native-entry adapter.
 *
 * [ARCH-02](../../../spec/implementation/compiler.md#arch-02) keeps this layer at
 * orchestration level. [STACK-01](../../../spec/implementation/compiler.md#stack-01)
 * preserves label transfers, explicit return metadata and native-entry boundaries.
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) permits the
 * contiguous helper implementation only with the same source/install/output order
 * and no interior entry into a validated replacement group.
 * [LIST-09](../../../spec/language/lists.md#list-09) executes append/insert input/preflight/commit prefixes once before ordered outputs.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) retains exact
 * header roots and releases all header targets before either truth successor.
 */
#include "codegen/internal.h"
#include "emission/emission.h"
#include "lists/lists.h"

/** Emit one validated node and its explicit successor; terminal writers own transfer.
 * Borrow the model/type/IR through the call. Stream or model failure returns false;
 * partial bytes remain in the caller's unpublished artifact buffer.
 * Component contract: [ARCH-02](../../../spec/implementation/compiler.md#arch-02), [STACK-01](../../../spec/implementation/compiler.md#stack-01), [CALL-08](../../../spec/language/calls-and-results.md#call-08), [LIST-09](../../../spec/language/lists.md#list-09), [FLOW-04](../../../spec/language/control-flow.md#flow-04). */
bool codegen_emit_control(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type, const ControlNode *node) {
    const char *name = codegen_control_name(scaffold, type, node);
    bool record = node->operation != CONTROL_REFUSAL_SNAPSHOT && node->operation != CONTROL_YIELD;
    if (name == NULL || !codegen_control_location(output, node->selector, node->site, record)) { return false; }
    bool written = true;
    switch (node->operation) {
        case CONTROL_COMPLETE: return codegen_control_complete(output);
        case CONTROL_ACQUIRE_INSTANCE:
            written = node->statement->enclosing_operation != NULL
                ? codegen_emit_tree_control(output, type, node, name)
                : codegen_control_acquire(output, name, node->alternative->selector,
                    node->statement->syntax->as.call.mode == AST_ENTRY_CALL); break;
        case CONTROL_ACQUIRE_LIST: case CONTROL_LIST_METHOD_PREFLIGHT:
            written = codegen_list_operation(scaffold, output, type, node, name); break;
        case CONTROL_SET_RETURN: written = codegen_control_connect(output, node->resume->selector, node->selector); break;
        case CONTROL_NATIVE_ENTRY: written = codegen_emit_native_entry(output, type); break;
        case CONTROL_REFUSAL_SNAPSHOT: written = codegen_control_refusal(output); break;
        case CONTROL_RELEASE_TARGET:
            written = node->statement->enclosing_operation != NULL
                    || node->statement->operation_kind != SEMANTIC_INSTANCE_OPERATION
                ? codegen_emit_tree_control(output, type, node, name) : codegen_control_release(output); break;
        case CONTROL_ABANDON_CHAIN: written = codegen_emit_tree_control(output, type, node, name); break;
        case CONTROL_RELEASE_CHAIN: written = codegen_emit_tree_control(output, type, node, name); break;
        case CONTROL_TRANSFER_OUTPUT:
            written = node->binding->intrinsic != NULL ? codegen_list_output(output, type, node, name)
                : codegen_control_output(output, name, node->statement->syntax->as.call.mode == AST_CONSTANT_READ); break;
        case CONTROL_SELECT_RECEIVER: written = codegen_control_assignment(output, name); break;
        case CONTROL_CONDITION:
            return node->chain != NULL && node->chain->header != NULL
                ? codegen_control_condition_result(output, name, node->next->selector, node->alternative->selector)
                : codegen_control_condition(output, name, node->next->selector, node->alternative->selector);
        case CONTROL_CAPTURE_ERROR: case CONTROL_CLEANUP_FAILURE: case CONTROL_FAILURE:
            return codegen_control_failure(output);
        case CONTROL_SAVE_POSITION: written = codegen_control_save(output, node->resume->selector); break;
        case CONTROL_YIELD: return codegen_control_yield(output);
        case CONTROL_RESUME_INSTANCE: return codegen_control_resume(output);
        case CONTROL_RESUME_SAVED: return codegen_control_resume(output);
        case CONTROL_ENTER_INSTANCE: written = codegen_control_restart(output); break;
        case CONTROL_FOREACH_ENTER: case CONTROL_FOREACH_NEXT: case CONTROL_FOREACH_LEAVE: case CONTROL_FOREACH_EMPTY:
        case CONTROL_CLEANUP_SCOPES: case CONTROL_RESET_SCOPES: case CONTROL_VALIDATE_TRAVERSALS:
            return codegen_emit_traversal_control(scaffold, output, type, node);
        /* The current backend executes the contiguous preparation prefix in
         * ama_program_prepare at acquisition and each replacement in its lead
         * runtime helper. These explicit audit phases must not repeat effects.
         * [CALL-08](../../../spec/language/calls-and-results.md#call-08) fixes
         * their order; validated groups have no interior branch/resume entry. */
        case CONTROL_RETIRE_RETURN:
        /* The contiguous selected acquisition helper performs selection once;
         * the explicit action/selection labels introduce no second evaluation. */
        case CONTROL_SELECT_RESULT: case CONTROL_ACTION_NEXT: case CONTROL_HEADER_ENTER:
        /* The method preflight label owns its entire contiguous typed prefix;
         * these audit stages introduce no second source/constructor/mutation. */
        case CONTROL_STAGE_METHOD_INPUT: case CONTROL_LIST_METHOD_PREPARE: case CONTROL_LIST_METHOD_COMMIT:
        case CONTROL_PREFLIGHT_INPUTS:
        case CONTROL_EVALUATE_INPUT: case CONTROL_INSTALL_PARAMETER: case CONTROL_INITIALIZE_RESULT:
        case CONTROL_SELECT_SOURCE: case CONTROL_PREFLIGHT_REPLACEMENT: case CONTROL_PREPARE_REPLACEMENT:
        case CONTROL_COMMIT_REPLACEMENT: case CONTROL_RELEASE_VALUES:
            break;
        default:
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "Unknown resolved control operation", NULL);
    }
    return written && codegen_control_jump(output, node->next == NULL, node->next == NULL ? 0 : node->next->selector);
}
