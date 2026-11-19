/** @file
 * @brief Retain one exact creation-path use before associating its canonical key.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves every incoming path.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
#include <stdint.h>
#include "../internal.h"

/** Internal caller has validated the guard-published path; no ancestry is followed
 * or invented here. New records remain owned if later key/name preparation fails.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11).
 */
bool static_registry_retain(StaticRegistryPlan *plan, const StaticCreationPath *path,
    StaticRegistryUse **output) {
    StaticRegistryUse *use;
    for (use = plan->first_use; use; use = use->next) {
        if (use->path == path) { break; }
    }
    if (use == NULL) {
        if (plan->use_count == SIZE_MAX) {
            return static_registry_fail(plan, STATIC_REGISTRY_OUT_OF_MEMORY, path->site,
                "Specialization use count exceeds representable storage");
        }
        use = static_registry_allocate(plan, sizeof(*use));
        if (use == NULL) { return false; }
        *use = (StaticRegistryUse){.path = path, .ordinal = plan->use_count, .next = plan->first_use};
        plan->first_use = use;
        plan->use_count++;
    }
    *output = use;
    return true;
}
