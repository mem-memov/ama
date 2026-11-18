/** @file
 * @brief Close one producer using monotone exact evidence rather than pass budgets or inverse guessing.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires complete producers.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits structural equality inference.
 */
#include "../internal.h"

/** New equations, independent metadata, explicit requirements and checked
 * source contexts justify another attempt. Snapshot/canonical-term allocation
 * counts are not evidence; actual contexts enable missing scoped definition reads.
 * Selected value closure avoids demanding unrelated compilation roots.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids false progress.
 */
bool static_publication_close(StaticPublicationPlan *plan, StaticResultUse *use, const StaticTerm **output) {
    StaticConstraintSolver *solver = plan->results->guards->applications->solver;
    for (;;) {
        size_t equations = solver->equation_count, metadata = solver->metadata_count,
            requirements = solver->requirement_count, contexts = plan->results->context_count;
        StaticResultOutcome outcome;
        if (!static_result_evaluate(plan->results, use->context, use->source->expression, &outcome)) {
            (void)static_publication_dependencies(plan); return false;
        }
        bool dependencies_closed;
        if (!static_publication_dependencies_closed(plan, use, &dependencies_closed)) { return false; }
        const StaticTerm *value;
        StaticConstraintOutcome closed;
        if (!static_constraint_value(solver, use->source->value, &value, &closed)) {
            (void)static_publication_dependencies(plan); return false;
        }
        if (outcome == STATIC_RESULT_READY && dependencies_closed && closed == STATIC_CONSTRAINT_COMPLETE) {
            *output = value;
            return true;
        }
        if (equations == solver->equation_count && metadata == solver->metadata_count &&
            requirements == solver->requirement_count && contexts == plan->results->context_count) {
            return static_publication_fail(plan, STATIC_PUBLICATION_UNRESOLVED, use->source->expression->span,
                "Static producer or its required result dependencies remain unresolved");
        }
    }
}
