/** @file
 * @brief Reify complete keys and recheck cached paths only after genuine finite evidence changes.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates merged creation paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves symbolic environments.
 */
#include "internal.h"

/** Counts are exact monotone fact identities. Dirty solving or a new occurrence
 * prevents reuse even if a caller already solved new equations externally.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) rejects no-change retry loops.
 */
static bool unchanged(const StaticGuardPlan *plan) {
    const StaticConstraintSolver *solver = plan->applications->solver;
    return plan->refreshed && !solver->dirty && plan->observed_paths == plan->path_count &&
        plan->observed_equations == solver->equation_count && plan->observed_metadata == solver->metadata_count &&
        plan->observed_requirements == solver->requirement_count &&
        plan->observed_definitions == plan->definition_count &&
        plan->observed_definition_edges == plan->definition_edge_count &&
        plan->observed_definition_paths == plan->definition_path_count;
}

/** Normalization adds no variable equations or definitions. New metadata is the
 * only refinement made by its value queries; each repeating pass therefore has
 * a genuine monotone fact, not a depth/time/iteration budget.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates concrete paths.
 */
bool static_guard_refresh(StaticGuardPlan *plan, StaticGuardClosure *output) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (output == NULL) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Missing static guard refresh outcome receiver");
    }
    if (unchanged(plan)) { *output = plan->closure; return true; }
    StaticConstraintSolver *solver = plan->applications->solver;
    StaticGuardClosure closure;
    size_t metadata;
    do {
        metadata = solver->metadata_count;
        if (!static_guard_creation_keys(plan, &closure)) { return false; }
        if (!static_guard_definition_keys(plan, &closure)) { return false; }
    } while (metadata != solver->metadata_count);
    if (!static_guard_edges(plan) || !static_guard_creation_graph(plan) ||
        !static_guard_definition_graph(plan)) { return false; }
    plan->observed_paths = plan->path_count;
    plan->observed_equations = solver->equation_count;
    plan->observed_metadata = solver->metadata_count;
    plan->observed_requirements = solver->requirement_count;
    plan->observed_definitions = plan->definition_count;
    plan->observed_definition_edges = plan->definition_edge_count;
    plan->observed_definition_paths = plan->definition_path_count;
    plan->closure = closure;
    plan->refreshed = true;
    *output = closure;
    return true;
}
