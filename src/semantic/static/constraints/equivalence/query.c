/** @file
 * @brief Prove current symbolic term congruence without equating independent unknowns or unfolding definitions.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) uses exact finite nominal equality.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) still requires closed keys before repeated expansion.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) releases all query scratch.
 */
#include "../internal.h"

/** Query-local unordered pair; terms/representatives remain immutable during
 * this solved-facts traversal. The finite queue also records visited pairs.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves shared DAG structure.
 */
typedef struct EquivalentPair {
    struct EquivalentPair *next;
    const StaticTerm *left;
    const StaticTerm *right;
} EquivalentPair;

/** Normalize by existing representatives, deduplicate exact unordered pairs and
 * queue once. This adds no mathematical equality. O(P) lookup for P seen pairs;
 * scratch refusal leaves all ownership in the caller's local arena.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) never guesses unknown-variable equality.
 */
static bool enqueue(StaticConstraintSolver *solver, SourceArena *scratch, EquivalentPair **head,
    EquivalentPair **tail, const StaticTerm *left, const StaticTerm *right) {
    left = static_constraint_root(solver, left);
    right = static_constraint_root(solver, right);
    if (left == right) { return true; }
    if (left->ordinal > right->ordinal) { const StaticTerm *swap = left; left = right; right = swap; }
    for (const EquivalentPair *pair = *head; pair; pair = pair->next) {
        if (pair->left == left && pair->right == right) { return true; }
    }
    EquivalentPair *pair = source_arena_allocate(scratch, sizeof(*pair));
    if (pair == NULL) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, left->site,
            "Symbolic congruence query could not allocate pair scratch", left, right);
    }
    *pair = (EquivalentPair){.left = left, .right = right};
    if (*tail) { (*tail)->next = pair; }
    else { *head = pair; }
    *tail = pair;
    return true;
}

/** Distinct variables are not proven equal; matching nominal constructors
 * require the exact original fixed/residual partitions and every captured value.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) retains unused bindings and local residuals.
 */
static bool nominal(const EquivalentPair *pair) {
    if (pair->left->kind != STATIC_TERM_NOMINAL || pair->right->kind != STATIC_TERM_NOMINAL) { return false; }
    const StaticNominalTerm *left = &pair->left->as.nominal, *right = &pair->right->as.nominal;
    if (left->origin != right->origin || left->capture_count != right->capture_count || left->residual_count != right->residual_count) { return false; }
    const StaticTermCapture *capture = right->first_capture;
    for (const StaticTermCapture *item = left->first_capture; item; item = item->next, capture = capture->next) {
        if (item->formal != capture->formal) { return false; }
    }
    const StaticTermResidual *residual = right->first_residual;
    for (const StaticTermResidual *item = left->first_residual; item; item = item->next, residual = residual->next) {
        if (item->formal != residual->formal) { return false; }
    }
    return true;
}

/** Solve only existing evidence, then visit each distinct substituted pair once.
 * False is an absence of proof, so later legitimate anchors may make it true.
 * Free-term structural induction gives exact congruence; the queue terminates
 * on finite owned DAG pairs. Time O(P^2 + C), scratch O(P) for P visited pairs
 * and C compared capture/residual links, plus the existing solver's own cost.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) keeps inference independent of runtime values.
 */
bool static_constraint_equivalent(StaticConstraintSolver *solver, const StaticTerm *left,
    const StaticTerm *right, bool *output) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    if (output == NULL || !static_term_has_value(solver->terms, left) || !static_term_has_value(solver->terms, right)) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_INVALID, (SourceSpan){0}, "Symbolic congruence requires owned terms and a receiver", NULL, NULL);
    }
    StaticConstraintOutcome outcome;
    if (!static_constraint_solve(solver, &outcome)) { return false; }
    SourceArena scratch;
    source_arena_init(&scratch, solver->arena.allocator);
    EquivalentPair *head = NULL, *tail = NULL;
    bool success = enqueue(solver, &scratch, &head, &tail, left, right), equivalent = true;
    for (const EquivalentPair *pair = head; success && equivalent && pair; pair = pair->next) {
        equivalent = nominal(pair);
        if (!equivalent) { break; }
        const StaticTermCapture *other = pair->right->as.nominal.first_capture;
        for (const StaticTermCapture *item = pair->left->as.nominal.first_capture; success && item; item = item->next, other = other->next) {
            success = enqueue(solver, &scratch, &head, &tail, item->value, other->value);
        }
    }
    source_arena_destroy(&scratch);
    if (!success) { return false; }
    *output = equivalent;
    return true;
}
