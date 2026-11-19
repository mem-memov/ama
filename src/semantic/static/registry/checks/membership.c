/** @file
 * @brief Validate actual registry-owned use handles before following their scoped paths.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains distinct incoming uses.
 */
#include "../internal.h"

/** Exact identity excludes copied or unrelated handles without dereferencing them.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11).
 */
bool static_registry_has_use(const StaticRegistryPlan *plan, const StaticRegistryUse *use) {
    for (const StaticRegistryUse *item = plan->first_use; item; item = item->next) {
        if (item == use) { return true; }
    }
    return false;
}
