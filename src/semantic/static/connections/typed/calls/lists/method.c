/** @file
 * @brief Normalize a fixed intrinsic method under its original list-service parent.
 * [LIST-04](../../../../../../../spec/language/lists.md#list-04) fixes methods and forbids their use as values.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../../../bindings/normalize.h"

/** Reuse the canonical descriptor table and binding normalizer rather than
 * inventing source declarations, method instances or an action constant table.
 * Element-reference and exact integer Type terms come from this service parent.
 * [LIST-04](../../../../../../../spec/language/lists.md#list-04) defines source-independent method interfaces.
 */
bool static_typed_list_method(StaticConnectionPlan *plan, const StaticTypedOperation *parent,
    const AstNode *node, const StaticTypedOperation **output) {
    const AstAccess *selector = node->as.call.target;
    SemanticListMethod method;
    if (selector->reference || selector->whole_list || selector->index || node->as.call.mode != AST_ENTRY_CALL ||
        !semantic_list_method(selector->name, &method)) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, selector->span, "List method requires its plain declared name and entry mode");
    }
    BindingPlan *bindings = plan->constants->publications->results->guards->applications->bindings;
    BindingInterface *view;
    BindingApplication *application;
    if (!binding_list_interface(bindings, method, &view) || !binding_normalize_runtime(bindings, view, node, &application)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (plan->operation_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, node->span, "List method metadata count exceeds representable storage");
    }
    StaticTypedOperation *operation = static_connection_allocate(plan, sizeof(*operation));
    if (!operation) { return false; }
    *operation = (StaticTypedOperation){.next = plan->first_operation, .caller = parent->caller, .syntax = node,
        .bindings = application, .kind = STATIC_OPERATION_LIST_METHOD, .element = parent->element,
        .integer = parent->integer, .reference_elements = parent->reference_elements, .method = method};
    plan->first_operation = operation;
    plan->operation_count++;
    *output = operation;
    return true;
}
