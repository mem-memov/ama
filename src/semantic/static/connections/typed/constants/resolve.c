/** @file
 * @brief Add qualified constant transfer Type facts after independent namespace validation.
 * [CONST-03](../../../../../../spec/language/declarations.md#const-03) requires fixed concrete namespace selection.
 * [TYPE-05](../../../../../../spec/language/declarations.md#type-05) requires exact owned scalar/list transfer forms.
 */
#include "../../internal.h"

/** Live plan membership precedes dereferencing supplied transfer identities.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) preserves owner boundaries.
 */
static StaticTypedConstant *registered(const StaticConnectionPlan *plan, const StaticTypedConstant *candidate) {
    for (StaticTypedConstant *item = plan->first_constant; item; item = item->next) {
        if (item == candidate) { return item; }
    }
    return NULL;
}

/** The caller supplies an exact actual namespace context; no context-name search
 * or default root is invented. Member Type equations may refine the destination
 * only after that namespace is independently fixed. The first checked namespace/storage transfer
 * is a new finite proof fact even if another consumer supplied every equality;
 * repeated validation reports no proof progress. Constant values never run.
 * [CONST-03](../../../../../../spec/language/declarations.md#const-03) separates namespace selection and transfer inference.
 */
bool static_connection_constant(StaticConnectionPlan *plan, const StaticTypedConstant *transfer,
    const StaticResultContext *actual, bool *changed) {
    if (!static_connection_dependencies(plan)) { return false; }
    StaticTypedConstant *record = registered(plan, transfer);
    if (!changed || !record) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid qualified transfer identity or progress receiver");
    }
    StaticConstraintSolver *solver = plan->constants->publications->results->guards->applications->solver;
    size_t before = solver->equation_count;
    const StaticConstantValue *value;
    if (!static_constant_value(plan->constants, transfer->source, actual, &value)) {
        (void)static_connection_dependencies(plan); return false;
    }
    if (transfer->reference || transfer->list != value->declaration->list) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, transfer->source->occurrence->source->span,
            "Qualified constant transfer storage form differs from its destination");
    }
    bool annotation_changed;
    if (!static_ordinary_equations(plan->constants->publications, value->declaration->source, &annotation_changed) ||
        !static_constraint_equate(solver, transfer->type, value->declaration->type, transfer->source->occurrence->source->span)) {
        (void)static_connection_dependencies(plan); return false;
    }
    bool proof_changed = !record->equations_ready;
    record->equations_ready = true;
    *changed = proof_changed || solver->equation_count != before;
    return true;
}
