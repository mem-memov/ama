/** @file
 * @brief Handoff prepared source operands to exact actual-origin application and creation guards.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves actual capture maps.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) guards before producer expansion.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains pending source operators.
 */
#include "../internal.h"

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
    if (!static_result_prepare(plan, use) || !static_result_application(plan, use->source, use->prepared, &use->application)) { return false; }
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
