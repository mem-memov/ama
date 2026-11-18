/** @file
 * @brief Own source/generated static input snapshots without renumbering written positions.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves explicit skips.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial links.
 */
#include <stdint.h>
#include "../internal.h"

/** A generated entry may follow a written skip of the same formal; only its
 * nonnull value supplies that binding. No source counter is advanced here.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates generation from positions.
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
