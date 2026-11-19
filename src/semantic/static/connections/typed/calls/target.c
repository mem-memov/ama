/** @file
 * @brief Select exact ordinary target metadata without evaluating target values.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) restricts selected names to public results.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) retains caller lexical use identity.
 */
#include "internal.h"

/** A selected operation sees its parent's original result interface only. Its
 * index operands remain caller-scoped lexical uses; no private field or equally
 * spelled caller variable can repair an unknown or missing parent result.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) requires exact storage suffixes.
 */
static bool selected(StaticConnectionPlan *plan, const StaticTypedScope *scope,
    const AstNode *node, StaticTypedOperand *output, StaticResultOutcome *outcome) {
    const StaticTypedOperation *parent = NULL;
    for (const StaticTypedOperation *item = plan->first_operation; item; item = item->next) {
        if (item->caller == scope && item->syntax == node->as.call.enclosing_operation) { parent = item; break; }
    }
    if (!parent) { *outcome = STATIC_RESULT_PENDING; return true; }
    if (parent->kind == STATIC_OPERATION_LIST_METHOD) {
        if (!static_typed_list_result(plan, parent, node, output)) { return false; }
        *outcome = STATIC_RESULT_READY;
        return true;
    }
    if (parent->kind != STATIC_OPERATION_SOURCE) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, node->span, "List service target must select an intrinsic method");
    }
    const AstAccess *selector = node->as.call.target;
    for (const StaticTypedField *field = parent->target->first_field; field; field = field->next) {
        if (field->definition->role != STATIC_FIELD_RESULT || !static_same_name(field->definition->name, selector->name)) { continue; }
        if (field->definition->reference != selector->reference ||
            field->definition->list != (selector->whole_list || selector->index != NULL)) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, selector->span, "Selected result storage differs from its declaration");
        }
        *output = (StaticTypedOperand){.type = field->type, .field = field->definition,
            .reference = field->definition->reference, .list = selector->whole_list};
        *outcome = STATIC_RESULT_READY;
        return true;
    }
    return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, selector->span, "No public result matches the selected operation target");
}

/** Root targets consume retained caller uses; selected targets consume only the
 * parent's public result metadata. Whole lists defer to the intrinsic service
 * consumer instead of being mistaken for an ordinary element invocation.
 * [CALL-09](../../../../../../spec/language/calls-and-results.md#call-09) preserves direct result execution.
 */
bool static_typed_target(StaticConnectionPlan *plan, const StaticTypedScope *scope,
    const AstNode *node, StaticTypedOperand *output, StaticResultOutcome *outcome) {
    const StaticTypedField *field = NULL;
    if (node->as.call.enclosing_operation) {
        return selected(plan, scope, node, output, outcome);
    } else {
        for (const StaticRuntimeUse *use = scope->runtime->first_use; use; use = use->next) {
            if (use->statement == node && use->syntax == node->as.call.target) {
                if (!static_connection_field(plan, scope, use->field, &field)) { return false; }
                break;
            }
        }
        if (!field) {
            return static_connection_fail(plan, STATIC_CONNECTION_INVALID, node->span, "Operation target lacks its original lexical use");
        }
    }
    *output = (StaticTypedOperand){.type = field->type, .field = field->definition,
        .reference = field->definition->reference, .list = node->as.call.target->whole_list};
    *outcome = STATIC_RESULT_READY;
    return true;
}
