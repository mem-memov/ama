/** @file
 * @brief Validate actual registered ordinary-Type handles before dereferencing callers' pointers.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) fixes source/context identity.
 */
#include "internal.h"

/** Exact owner membership never substitutes an equal-looking copied record.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) preserves actual occurrences.
 */
bool static_ordinary_has(const StaticPublicationPlan *plan, const StaticOrdinaryType *candidate) {
    for (const StaticOrdinaryType *type = plan->first_type; type; type = type->next) {
        if (type == candidate) { return true; }
    }
    return false;
}
