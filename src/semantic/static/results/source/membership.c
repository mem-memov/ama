/** @file
 * @brief Validate source-record identity before operand or producer preparation follows its payload.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) requires exact lexical identity.
 */
#include "../internal.h"

/** Pointer membership excludes copied and foreign records without dereferencing them.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08).
 */
bool static_result_has_source(const StaticResultPlan *plan, const StaticResultSource *source) {
    for (const StaticResultSource *item = plan->first_source; item; item = item->next) {
        if (item == source) { return true; }
    }
    return false;
}
