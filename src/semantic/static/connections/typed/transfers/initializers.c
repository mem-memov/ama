/** @file
 * @brief Constrain declaration/default initializer Types without constructing values.
 * [TYPE-02](../../../../../../spec/language/declarations.md#type-02) makes every list initializer one scalar element.
 * [TYPE-05](../../../../../../spec/language/declarations.md#type-05) forbids implicit ownership conversion and list splicing.
 */
#include "../../internal.h"

/** Select the original declaration field rather than resolving its spelling
 * against later receiver introductions or body-local cells.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes lexical identities first.
 */
static const StaticTypedField *declaration(const StaticTypedScope *scope, const AstNode *node) {
    for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
        if (field->definition->statement == node && field->definition->type_expression) { return field; }
    }
    return NULL;
}

/** One list item uses the scalar form of its destination element. Type facts
 * constrain even defaults that a call may replace, without executing defaults.
 * A literal supplies only a capability obligation, never a Type equation.
 * [STATE-02](../../../../../../spec/language/declarations.md#state-02) preserves default ordering independently of solving.
 */
static bool initializer(StaticConnectionPlan *plan, const StaticResultContext *actual, const StaticTypedScope *scope,
    const StaticRuntimeEvent *event, bool *ready) {
    const StaticTypedField *field = declaration(scope, event->node);
    if (!field) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, event->site, "Initializer lacks its original typed declaration");
    }
    StaticTypedOperand operand;
    StaticResultOutcome outcome;
    if (!static_typed_operand(plan, scope, event, event->source, &operand, &outcome)) { return false; }
    if (outcome == STATIC_RESULT_PENDING) {
        if (event->source->kind == AST_LITERAL_SOURCE && field->definition->reference) {
            return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, event->site, "Literal initializer requires owned scalar element storage");
        }
        bool resolved;
        if (!static_typed_defer_constant(plan, actual, event->source, field->type, field->definition->reference, false, &resolved)) { return false; }
        *ready &= resolved;
        return true;
    }
    if (operand.list || operand.reference != field->definition->reference) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, event->site, "Initializer source must match scalar element storage");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    if (!static_constraint_equate(solver, field->type, operand.type, event->site)) {
        (void)static_connection_dependencies(plan); return false;
    }
    return true;
}

/** Traverse finite original initializer events, retaining Pending capability and
 * namespace obligations while accepting unrelated Type equations. No default
 * values execute and no field/body validation is certified by this pass.
 * [TPL-10](../../../../../../spec/language/templates.md#tpl-10) propagates exact finite evidence.
 */
bool static_connection_initializers(StaticConnectionPlan *plan, const StaticResultContext *actual,
    bool *changed, StaticResultOutcome *outcome) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!changed || !outcome) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing initializer progress or outcome receiver");
    }
    const StaticTypedScope *scope;
    if (!static_connection_context(plan, actual, &scope)) { return false; }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    bool ready = true;
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind == STATIC_RUNTIME_INITIALIZER && !initializer(plan, actual, scope, event, &ready)) { return false; }
    }
    *changed = solver->equation_count != before;
    *outcome = ready ? STATIC_RESULT_READY : STATIC_RESULT_PENDING;
    return true;
}
