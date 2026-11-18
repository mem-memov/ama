/** @file
 * @brief Handoff prepared source operands to exact actual-origin application and creation guards.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves actual capture maps.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) guards before producer expansion.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains pending source operators.
 */
#include "../internal.h"
#include "../../constraints/internal.h"

/** Unknown-origin snapshots cannot be rechecked into a different interface.
 * Replace one only when the head has independently acquired a nominal origin;
 * otherwise retain its original request provenance without manufacturing work.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) forbids origin guessing.
 */
static bool application(StaticResultPlan *plan, StaticResultUse *use) {
    StaticApplicationPlan *applications = plan->guards->applications;
    StaticConstraintOutcome solved;
    if (!static_constraint_solve(applications->solver, &solved)) {
        (void)static_result_dependencies(plan); return false;
    }
    const StaticTerm *head = static_constraint_root(applications->solver, use->source->head);
    const StaticTerm *result = static_constraint_root(applications->solver, use->source->value);
    StaticCategory category;
    const StaticSignature *signature;
    static_constraint_description(applications->solver, use->source->head, &category, &signature);
    (void)signature;
    if (head->kind == STATIC_TERM_VARIABLE && result->kind == STATIC_TERM_NOMINAL &&
        (category == STATIC_CATEGORY_TEMPLATE ||
            (category == STATIC_CATEGORY_TYPE && use->prepared->input_count == 0))) {
        const StaticApplicationRecord *record;
        if (!static_application_recover(applications, use->prepared, result, &record)) {
            (void)static_result_dependencies(plan); return false;
        }
        use->application = record;
        return true;
    }
    bool newly_known = use->application != NULL && use->application->pending_origin &&
        static_constraint_root(applications->solver, use->source->head)->kind == STATIC_TERM_NOMINAL;
    if (use->application == NULL || newly_known) {
        const StaticApplicationRecord *record;
        if (!static_application_source(applications, use->prepared, &record)) {
            (void)static_result_dependencies(plan); return false;
        }
        use->application = record;
    }
    return true;
}

/** Guard entry is separate from output evaluation: plain Type reuse and partial
 * constructors have no producer context. Preserve the checked creation identity
 * for subsequent checks and retain Pending without claiming result closure.
 * A structurally complete application can still have unresolved creation
 * ancestry; propagate that Pending rather than publishing a false ready proof.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves required paths.
 */
bool static_result_apply(StaticResultPlan *plan, StaticResultUse *use, StaticApplicationOutcome *output) {
    if (!static_result_dependencies(plan)) { return false; }
    if (output == NULL || !static_result_has_use(plan, use)) {
        return static_result_fail(plan, STATIC_RESULT_INVALID, (SourceSpan){0}, "Invalid static application use or outcome receiver");
    }
    if (!static_result_prepare(plan, use) || !application(plan, use)) { return false; }
    StaticApplicationOutcome checked;
    if (!static_application_check(plan->guards->applications, use->application, &checked)) {
        (void)static_result_dependencies(plan); return false;
    }
    if (use->producer == NULL) {
        const StaticCreationPath *path = NULL;
        StaticCreationOutcome creation;
        if (!static_creation_enter(plan->guards, use->context->creation, use->application, &path, &creation)) {
            (void)static_result_dependencies(plan); return false;
        }
        if (creation == STATIC_CREATION_PENDING) { checked = STATIC_APPLY_PENDING; }
        if (creation == STATIC_CREATION_ENTERED && !static_result_context(plan, path, &use->producer)) { return false; }
    } else {
        StaticGuardClosure closure;
        if (!static_guard_refresh(plan->guards, &closure)) {
            (void)static_result_dependencies(plan); return false;
        }
    }
    *output = checked;
    return true;
}
