/** @file
 * @brief Retain intrinsic list-service Type metadata without constructing method instances.
 * [LIST-04](../../../../../../../spec/language/lists.md#list-04) fixes the service and standard integer interfaces.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Only the exact standard namespace supplies intrinsic position/length Types;
 * arbitrary bracket indices still retain their ordinary native protocol Type.
 * Source loading has already collected this dependency for a structural service.
 * [LIST-04](../../../../../../../spec/language/lists.md#list-04) fixes integer service ports.
 */
static const StaticModule *integer_module(const StaticInventory *inventory) {
    for (const StaticModule *module = inventory->first; module; module = module->next) {
        if (strcmp(module->definition->source->logical_path, "..integer") == 0) { return module; }
    }
    return NULL;
}

/** Unknown element Types may receive later equations from method sources or
 * outputs. Service preparation adds no default element, body or actual context.
 * [TPL-03](../../../../../../../spec/language/templates.md#tpl-03) permits inference through ordinary connections.
 */
bool static_typed_list_service(StaticConnectionPlan *plan, const StaticTypedScope *caller,
    const AstNode *node, const StaticTypedOperand *target, const StaticTypedOperation **output) {
    if (node->as.call.mode != AST_RETAINED_READ || node->as.call.input_count || node->as.call.output_count || !node->as.call.has_selected) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, node->span, "Whole-list service requires only selected entry methods");
    }
    StaticTermStore *terms = plan->constants->publications->results->guards->applications->terms;
    const StaticModule *integer = integer_module(terms->inventory);
    if (!integer || integer->parameter_count || integer->definition->syntax.sections.kind != MODULE_NATIVE) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, node->span, "List service requires the exact concrete native standard integer module");
    }
    const StaticTerm *integer_type;
    if (!static_term_nominal(terms, integer, NULL, 0, node->span, &integer_type)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (plan->operation_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, node->span, "List service metadata count exceeds representable storage");
    }
    StaticTypedOperation *operation = static_connection_allocate(plan, sizeof(*operation));
    if (!operation) { return false; }
    *operation = (StaticTypedOperation){.next = plan->first_operation, .caller = caller, .syntax = node,
        .kind = STATIC_OPERATION_LIST_SERVICE, .element = target->type, .integer = integer_type,
        .reference_elements = target->reference};
    plan->first_operation = operation;
    plan->operation_count++;
    *output = operation;
    return true;
}
