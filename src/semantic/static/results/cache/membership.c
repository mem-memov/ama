/** @file
 * @brief Validate exact definition-use ownership before reading cached scope/value metadata.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves scoped identities.
 */
#include "../internal.h"

/** Equal payload fields do not import a copied use into this live owner.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains original uses.
 */
bool static_result_has_definition_use(const StaticResultPlan *plan, const StaticResultDefinitionUse *use) {
    for (const StaticResultDefinitionUse *item = plan->first_definition_use; item; item = item->next) {
        if (item == use) { return true; }
    }
    return false;
}
