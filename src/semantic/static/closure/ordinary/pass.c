/** @file
 * @brief Compose finite ordinary equations before final actual field-Type validation.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) permits independent evidence to refine unknowns.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) requires real contexts and separate callee validation.
 */
#include "internal.h"
#include "../../publication/progress.h"

/** Defaults and assignments may remain Pending for literals or independent
 * namespaces; keep their equations and continue with foreach/call evidence.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) never executes defaults for inference.
 */
static bool transfers(StaticConnectionPlan *plan, const StaticResultContext *actual,
    StaticOrdinaryPass *report) {
    bool changed;
    StaticResultOutcome outcome;
    if (!static_connection_initializers(plan, actual, &changed, &outcome)) { return false; }
    report->changed |= changed;
    report->transfers_ready &= outcome == STATIC_RESULT_READY;
    if (!static_connection_assignments(plan, actual, &changed, &outcome)) { return false; }
    report->changed |= changed;
    report->transfers_ready &= outcome == STATIC_RESULT_READY;
    if (!static_connection_foreach(plan, actual, &changed)) { return false; }
    report->changed |= changed;
    return true;
}

/** Publish only a completely observed pass. Logical annotation proofs are part
 * of progress even when an existing equation already entails their equality;
 * scratch/canonical term allocation alone is not progress. This operation has no
 * retry loop: whole-worklist closure and remaining required callees are separate.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires evidence-driven refinement.
 */
bool static_closure_ordinary_pass(StaticConnectionPlan *plan, const StaticResultContext *actual,
    StaticOrdinaryPass *output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (!output) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Missing ordinary refinement report");
    }
    StaticPublicationPlan *publications = plan->constants->publications;
    StaticPublicationProgress before = static_publication_progress(publications);
    const StaticTypedScope *scope;
    bool changed;
    StaticOrdinaryPass report = {.transfers_ready = true};
    if (!static_connection_context(plan, actual, &scope) ||
        !static_connection_annotations(plan, scope, &changed)) { return false; }
    report.changed |= changed;
    if (!transfers(plan, actual, &report) ||
        !static_closure_ordinary_calls(plan, actual, scope, &report) ||
        !static_connection_merge_fields(plan, &changed)) { return false; }
    report.changed |= changed;
    StaticResultOutcome fields;
    if (!static_connection_validate_fields(plan, actual, &fields)) { return false; }
    report.fields_closed = fields == STATIC_RESULT_READY;
    report.changed |= !static_publication_same_progress(before, static_publication_progress(publications));
    *output = report;
    return true;
}
