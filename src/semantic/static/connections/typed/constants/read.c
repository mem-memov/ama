/** @file
 * @brief Connect explicit shared reads only after independent checked namespace selection.
 * [CONST-03](../../../../../../spec/language/declarations.md#const-03) separates namespace closure from output inference.
 * [CALL-08](../../../../../../spec/language/calls-and-results.md#call-08) preserves exact saved receiver forms.
 */
#include "../../internal.h"

/** Original call membership is checked before the supplied node is inspected.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) retains original source identities.
 */
static bool has_read(const StaticTypedScope *scope, const AstNode *node) {
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind == STATIC_RUNTIME_CALL && event->node == node) { return node->as.call.mode == AST_CONSTANT_READ; }
    }
    return false;
}

/** Each selected own constant contributes only its declared scoped Type and
 * scalar/list form. Qualified member selection has already validated labels and
 * positions independently of these caller receiver constraints.
 * [CONST-06](../../../../../../spec/language/declarations.md#const-06) permits dependent constant Type equations.
 */
static bool member(StaticConnectionPlan *plan, const StaticTypedScope *scope,
    const AstNode *node, const StaticConstantMember *selected) {
    const AstArgument *argument = selected->request->syntax.runtime_binding;
    StaticTypedOperand operand;
    if (!static_typed_receiver_operand(plan, scope, node, argument->receiver, &operand)) { return false; }
    if (operand.reference || operand.list != selected->declaration->list) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, argument->span, "Saved constant receiver storage differs from the selected declaration");
    }
    bool changed;
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    if (!static_ordinary_equations(plan->constants->publications, selected->declaration->source, &changed) ||
        !static_constraint_equate(solver, operand.type, selected->declaration->type, argument->span)) {
        (void)static_connection_dependencies(plan); return false;
    }
    return true;
}

/** Registration does not infer the namespace from requested member names or
 * receiver Types. A caller-provided actual namespace enables exact selection;
 * subsequent Type equations still certify no native capabilities or values.
 * [CONST-03](../../../../../../spec/language/declarations.md#const-03) preserves independent namespace selection.
 */
bool static_connection_read(StaticConnectionPlan *plan, const StaticResultContext *actual,
    const AstNode *node, const StaticResultContext *namespace_context, bool *changed, StaticResultOutcome *outcome) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!changed || !outcome) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing explicit constant read progress or outcome receiver");
    }
    const StaticTypedScope *scope;
    if (!static_connection_context(plan, actual, &scope)) { return false; }
    if (!has_read(scope, node)) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Constant read is not an original caller event");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    const StaticConstantRead *read;
    if (!static_constant_read(plan->constants, actual, node, &read)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (!namespace_context) {
        *changed = solver->equation_count != before;
        *outcome = STATIC_RESULT_PENDING;
        return true;
    }
    const StaticConstantSelection *selection;
    if (!static_constant_select(plan->constants, read, namespace_context, &selection)) {
        (void)static_connection_dependencies(plan); return false;
    }
    for (const StaticConstantMember *selected = selection->first; selected; selected = selected->next) {
        if (!member(plan, scope, node, selected)) { return false; }
    }
    *changed = solver->equation_count != before;
    *outcome = STATIC_RESULT_READY;
    return true;
}
