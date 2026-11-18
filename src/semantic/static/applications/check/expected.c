/** @file
 * @brief Select expected slot metadata from independent context facts rather than actual values.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) forbids first-actual signature selection.
 */
#include "../internal.h"

/** Formal identity and application context distinguish otherwise identical names
 * in different templates or capture environments.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) fixes declared interfaces.
 */
const StaticSignature *static_application_expected(const StaticApplicationPlan *plan,
    const StaticApplicationRecord *record, const StaticSymbol *formal) {
    for (const StaticApplicationExpectation *fact = plan->first_expectation; fact; fact = fact->next) {
        if (fact->record == record && fact->formal == formal) { return fact->signature; }
    }
    return formal->signature;
}
