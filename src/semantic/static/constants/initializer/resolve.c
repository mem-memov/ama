/** @file
 * @brief Supply exact typed initializer equations while preserving unresolved static work.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) derives Type from typed sources only.
 * [TPL-03](../../../../../spec/language/templates.md#tpl-03) uses structural equality, never values.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failure state.
 */
#include "../internal.h"

/** Exact initializer membership precedes borrowing its source/context records.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) retains actual source obligations.
 */
static bool registered(const StaticConstantPlan *plan, const StaticConstantInitializer *candidate) {
    for (const StaticConstantInitializer *item = plan->first_initializer; item; item = item->next) {
        if (item == candidate) { return true; }
    }
    return false;
}

/** Equality is valid even while checked expressions remain Pending. Add only
 * the typed element-Type equation; ordinary storage/cardinality, initializer
 * visibility, constant capability and runtime construction retain their owners.
 * A literal supplies no equation and cannot complete an unknown parameter.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) separates static evidence and constant values.
 */
bool static_constant_initializer_resolve(StaticConstantPlan *plan, const StaticConstantInitializer *item,
    const StaticResultContext *namespace_context, StaticResultOutcome *output) {
    if (!static_constant_dependencies(plan)) { return false; }
    if (output == NULL || !registered(plan, item)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid shared initializer obligation or outcome receiver");
    }
    const StaticConstantDeclaration *source = item->local;
    if (item->qualified) {
        const StaticConstantValue *value;
        if (!static_constant_value(plan, item->qualified, namespace_context, &value)) { return false; }
        source = value->declaration;
    }
    StaticResultPlan *results = plan->publications->results;
    StaticConstraintSolver *solver = results->guards->applications->solver;
    StaticResultOutcome destination_work, source_work = STATIC_RESULT_READY;
    if (!static_ordinary_evaluate(plan->publications, item->declaration->source, &destination_work)) {
        (void)static_constant_dependencies(plan); return false;
    }
    if (source) {
        if (!static_ordinary_evaluate(plan->publications, source->source, &source_work) ||
            !static_constraint_equate(solver, item->declaration->type, source->type, item->syntax->span)) {
            (void)static_constant_dependencies(plan); return false;
        }
    }
    StaticConstraintOutcome solved, closed;
    const StaticTerm *type;
    StaticGuardClosure closure;
    if (!static_constraint_solve(solver, &solved) || !static_guard_refresh(results->guards, &closure) ||
        !static_constraint_value(solver, item->declaration->type, &type, &closed)) {
        (void)static_constant_dependencies(plan); return false;
    }
    *output = closed == STATIC_CONSTRAINT_COMPLETE && destination_work == STATIC_RESULT_READY && source_work == STATIC_RESULT_READY ?
        STATIC_RESULT_READY : STATIC_RESULT_PENDING;
    return true;
}
