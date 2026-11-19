/** @file
 * @brief Select ephemeral intrinsic list-method result Types without fake declarations.
 * [LIST-04](../../../../../../../spec/language/lists.md#list-04) fixes scalar reference/length outputs.
 * [CALL-09](../../../../../../../spec/language/calls-and-results.md#call-09) permits immediate result execution.
 */
#include "../internal.h"

/** Intrinsic descriptors define public output identity and form. No body/private
 * field lookup or method instance exists; the service retains element/integer
 * Type terms independently of whether runtime result selection later succeeds.
 * [LIST-04](../../../../../../../spec/language/lists.md#list-04) defines intrinsic result interfaces.
 */
bool static_typed_list_result(StaticConnectionPlan *plan, const StaticTypedOperation *parent,
    const AstNode *node, StaticTypedOperand *output) {
    const AstAccess *selector = node->as.call.target;
    size_t ordinal;
    const SemanticListField *field = semantic_list_name(parent->method, true, selector->name, &ordinal);
    if (!field || selector->whole_list || selector->index || selector->reference != field->element_reference) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, selector->span, "No intrinsic public result matches the selected name and scalar storage form");
    }
    *output = (StaticTypedOperand){.type = field->element_reference ? parent->element : parent->integer,
        .reference = field->element_reference};
    return true;
}
