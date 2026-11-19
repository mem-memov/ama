/** @file
 * @brief Retain causal variable definitions and earlier binding equations in a failed static equality diagnostic.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original definition/application sites after normalization.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) keeps immutable terms separate from finite substitution evidence.
 */
#include <assert.h>
#include "internal.h"

/** Reuse one owned attribution node per semantic fact; repeated paths cannot
 * reorder or mutate an already attached witness.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first errors without failure-path allocation.
 */
static void attach(StaticConstraintSolver *solver, DiagnosticContext *node, DiagnosticRelated related) {
    bool added = diagnostic_add_context(&solver->error, node, related);
    assert(added);
    (void)added;
}

/** Queue exact original variables once. The solver owns zero-initialized work
 * links, used only after its sticky conflict; no subsequent solve can reuse them.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains causal evidence without allocation or recursive traversal.
 */
static void enqueue(StaticConstraintSolver *solver, const StaticTerm *term,
    StaticConstraintVariable **work) {
    if (term->kind != STATIC_TERM_VARIABLE || term->ordinal >= solver->variable_capacity) { return; }
    StaticConstraintVariable *record = solver->variables[term->ordinal];
    if (!record || record->evidence_seen) { return; }
    record->evidence_seen = true;
    record->evidence_next = *work;
    *work = record;
}

/** Binding reasons form a causal graph, not merely a representative chain:
 * both original operands matter even when one was already normalized at bind
 * time. Each variable enters the worklist once, including cycles in the graph
 * of explanations, so finite retained evidence terminates without a depth cap.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) keeps equations immutable.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original definitions and receipts.
 */
void static_constraint_equation_context(StaticConstraintSolver *solver, const StaticConstraintEquation *equation) {
    if (solver->status != STATIC_CONSTRAINT_CONFLICT) { return; }
    StaticConstraintVariable *work = NULL;
    for (const StaticConstraintEquation *current = equation; current; current = current->parent) {
        enqueue(solver, current->left, &work);
        enqueue(solver, current->right, &work);
    }
    while (work) {
        StaticConstraintVariable *record = work;
        work = record->evidence_next;
        attach(solver, &record->definition_context,
            static_constraint_definition(record->variable, "original static variable definition"));
        if (record->value) { enqueue(solver, record->value, &work); }
        if (!record->reason) { continue; }
        DiagnosticRelated related = {{.span = record->reason->site},
            "earlier equality establishing this static value", NULL};
        attach(solver, &record->binding_context, related);
        for (const StaticConstraintEquation *cause = record->reason; cause; cause = cause->parent) {
            enqueue(solver, cause->left, &work);
            enqueue(solver, cause->right, &work);
        }
    }
}
