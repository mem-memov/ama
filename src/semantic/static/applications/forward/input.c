/** @file
 * @brief Own named source/generated static input snapshots without changing their identities.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves explicit named source bindings.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial links.
 */
#include <stdint.h>
#include "../internal.h"

/** Snapshot one supplied or generated map entry without changing source order.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) distinguishes omission
 * from a supplied value and keeps generated inference separate from residuals.
 */
bool static_application_input(StaticApplicationPlan *plan, StaticApplicationRecord *record, StaticApplicationInput input) {
    if (record->input_count == SIZE_MAX) {
        return static_application_fail(plan, STATIC_APPLY_OUT_OF_MEMORY, input.site,
            "Application input count exceeds representable storage", input.formal);
    }
    StaticApplicationInput *copy = static_application_allocate(plan, sizeof(*copy));
    if (copy == NULL) { return false; }
    *copy = input;
    copy->next = NULL;
    if (record->last != NULL) { record->last->next = copy; }
    else { record->first = copy; }
    record->last = copy;
    record->input_count++;
    return true;
}
