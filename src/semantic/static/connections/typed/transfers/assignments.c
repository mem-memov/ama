/** @file
 * @brief Submit assignment Type equalities without runtime execution or invented anchors.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes source and receiver storage forms.
 * [COPY-06](../../../../../../spec/language/values-and-references.md#copy-06) forbids direct continuation self-copy.
 */
#include "../../internal.h"

/** The retained receiver belongs to this original assignment; lexical freshness
 * and indexed selection have already been fixed independently of Type values.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) forbids later name rebinding.
 */
static const StaticRuntimeReceiver *receiver(const StaticTypedScope *scope, const AstNode *node) {
    for (const StaticRuntimeReceiver *item = scope->runtime->first_receiver; item; item = item->next) {
        if (item->statement == node && item->syntax == node->as.assignment.receiver) { return item; }
    }
    return NULL;
}

/** One finite original assignment supplies one symmetric Type equality, while
 * storage forms remain exact independent checks. Deferred sources provide no
 * equality and retain a Pending result for later constant/capability consumers.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) prohibits literal Type inference.
 */
static bool assignment(StaticConnectionPlan *plan, const StaticResultContext *actual, const StaticTypedScope *scope,
    const StaticRuntimeEvent *event, bool *ready) {
    const StaticRuntimeReceiver *target = receiver(scope, event->node);
    if (!target) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, event->site, "Assignment lacks its original lexical receiver");
    }
    const StaticTypedField *field;
    if (!static_connection_field(plan, scope, target->field, &field)) { return false; }
    const AstSource *source = event->node->as.assignment.source;
    StaticTypedOperand operand;
    StaticResultOutcome outcome;
    if (!static_typed_operand(plan, scope, event, source, &operand, &outcome)) { return false; }
    if (outcome == STATIC_RESULT_PENDING) {
        bool resolved;
        if (!static_typed_defer_constant(plan, actual, source, field->type,
            target->syntax->reference, target->syntax->whole_list, &resolved)) { return false; }
        *ready &= resolved;
        return true;
    }
    if (operand.reference != target->syntax->reference || operand.list != target->syntax->whole_list) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, source->span, "Assignment source and receiver storage forms differ");
    }
    if (source->kind == AST_CONTINUATION_SOURCE && !target->syntax->index && !source->access->index &&
        operand.field == target->field) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, source->span, "Continuation self-copy is forbidden");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    if (!static_constraint_equate(solver, field->type, operand.type, source->span)) {
        (void)static_connection_dependencies(plan); return false;
    }
    return true;
}

/** Only actual incoming contexts enable own-body equations. Repeated requests
 * are idempotent; all partial equations remain solver-owned on failure, and
 * neither progress nor readiness is exposed after a refusal.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) retains actual creation ancestry.
 */
bool static_connection_assignments(StaticConnectionPlan *plan, const StaticResultContext *actual,
    bool *changed, StaticResultOutcome *outcome) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!changed || !outcome) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing assignment progress or outcome receiver");
    }
    const StaticTypedScope *scope;
    if (!static_connection_context(plan, actual, &scope)) { return false; }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    bool ready = true;
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind == STATIC_RUNTIME_ASSIGNMENT && !assignment(plan, actual, scope, event, &ready)) { return false; }
    }
    *changed = solver->equation_count != before;
    *outcome = ready ? STATIC_RESULT_READY : STATIC_RESULT_PENDING;
    return true;
}
