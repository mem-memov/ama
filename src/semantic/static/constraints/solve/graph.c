/** @file
 * @brief Walk current substitution/capture graphs iteratively for occurs and closure queries.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) separates finite terms from definitions.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns visited scratch.
 */
#include "../internal.h"

/** A resolved vertex is visited once per query, including shared subgraphs.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires finite shared work.
 */
typedef struct ConstraintVisit {
    struct ConstraintVisit *next;
    struct ConstraintVisit *memo_next;
    const StaticTerm *term;
} ConstraintVisit;

/** Resolve aliases before memoization; source module/layout edges are never read.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) bounds structural traversal.
 */
static bool push(StaticConstraintSolver *solver, SourceArena *scratch, const StaticTerm *term,
    ConstraintVisit **work, ConstraintVisit **memo) {
    term = static_constraint_root(solver, term);
    for (const ConstraintVisit *item = *memo; item; item = item->memo_next) {
        if (item->term == term) { return true; }
    }
    ConstraintVisit *item = static_constraint_scratch(solver, scratch, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (ConstraintVisit){.term = term, .next = *work, .memo_next = *memo};
    *work = item;
    *memo = item;
    return true;
}

/** A nonnull needle requests occurs checking; null requests closure. Unknown
 * values do not suppress other known metadata conflicts in the same graph.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) distinguishes local binders.
 */
static bool walk(StaticConstraintSolver *solver, const StaticTerm *term,
    const StaticTerm *needle, bool *output) {
    SourceArena scratch;
    source_arena_init(&scratch, solver->arena.allocator);
    ConstraintVisit *work = NULL, *memo = NULL;
    bool success = push(solver, &scratch, term, &work, &memo);
    bool result = needle == NULL;
    while (success && work != NULL) {
        const StaticTerm *current = work->term;
        work = work->next;
        if (current == needle) { result = true; break; }
        if (current->kind == STATIC_TERM_VARIABLE) {
            if (needle == NULL) { result = false; }
            continue;
        }
        if (needle == NULL) {
            bool pending = false;
            if (!static_constraint_metadata_match(solver, current, current, current->site, &pending)) {
                success = false;
                break;
            }
            if (pending) { result = false; }
        }
        for (const StaticTermCapture *capture = current->as.nominal.first_capture; capture; capture = capture->next) {
            if (!push(solver, &scratch, capture->value, &work, &memo)) { success = false; break; }
        }
    }
    source_arena_destroy(&scratch);
    if (success) { *output = result; }
    return success;
}

/** Occurs checking visits fixed values, never residual formal declarations.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) rejects infinite substitution.
 */
bool static_constraint_contains(StaticConstraintSolver *solver, const StaticTerm *term,
    const StaticTerm *needle, bool *output) { return walk(solver, term, needle, output); }

/** A closed partial constructor may retain local residuals but no free variables.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires concrete bindings.
 */
bool static_constraint_closed(StaticConstraintSolver *solver, const StaticTerm *term,
    bool *output) { return walk(solver, term, NULL, output); }
