/** @file
 * @brief Reconstruct the unique preceding nominal map B minus explicitly applied P.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) retains unused
 * captures and every existing residual. [TPL-10](../../../../../spec/language/templates.md#tpl-10)
 * supplies the nominal origin from the actual result, never an expected signature.
 */
#include <stdint.h>
#include "internal.h"

bool static_recovery_constructor(StaticApplicationPlan *plan, const StaticTermApplication *source,
    const StaticTerm *head, const StaticTerm *actual, SourceSpan site) {
    if (!static_recovery_names(plan, source, actual)) { return false; }
    size_t maximum = actual->as.nominal.capture_count;
    if (maximum > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, site,
            "Recovered capture map exceeds representable storage", NULL);
    }
    StaticTermCapture *captures = maximum ? static_application_allocate(plan, maximum * sizeof(*captures)) : NULL;
    if (maximum != 0 && captures == NULL) { return false; }
    size_t count = 0;
    for (const StaticTermCapture *capture = actual->as.nominal.first_capture; capture; capture = capture->next) {
        if (static_recovery_supplied(source, capture->formal) == NULL) {
            captures[count++] = (StaticTermCapture){.formal = capture->formal, .value = capture->value};
        }
    }
    const StaticTerm *candidate;
    if (!static_term_nominal(plan->terms, actual->as.nominal.origin, captures, count, site, &candidate)
        || !static_constraint_equate(plan->solver, head, candidate, site)) {
        (void)static_application_dependencies(plan);
        return false;
    }
    return true;
}
