/** @file
 * @brief Connect complete known-Type requirements to actual guarded result contexts and registry dependencies.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) validates reused Types independently of constructor creation.
 */
#include "internal.h"

/** Guard-owned attributed paths preserve caller identity and original source.
 * Register the result context only after the selected Type closes; neither step
 * checks bodies or creates an implicit runtime instance.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves required provenance.
 */
bool static_closure_require(StaticClosurePlan *plan, const StaticResultContext *consumer,
    const StaticTerm *type, SourceSpan site, const StaticResultContext **output, bool *ready) {
    StaticResultPlan *results = plan->connections->constants->publications->results;
    const StaticCreationPath *path;
    StaticConstraintOutcome outcome;
    if (!static_guard_require_type(results->guards, consumer->creation, type, site, &path, &outcome)) {
        (void)static_closure_dependencies(plan); return false;
    }
    if (outcome == STATIC_CONSTRAINT_PENDING) { *ready = false; return true; }
    if (!static_result_context(results, path, output)) {
        (void)static_closure_dependencies(plan); return false;
    }
    *ready = true;
    return true;
}
