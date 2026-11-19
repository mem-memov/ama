/** @file
 * @brief Require every original field's scalar or list-element definition without executing defaults.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) requires concrete Types even for empty/unread storage.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves original field provenance.
 */
#include "internal.h"

/** Type reuse adds validation attribution; explicit constructor applications
 * were separately guarded by actual annotation evaluation in the ordinary pass.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates these relations.
 */
bool static_closure_fields(StaticClosurePlan *plan, const StaticClosureEntry *entry, bool *output) {
    bool ready = true;
    for (const StaticTypedField *field = entry->scope->first_field; field; field = field->next) {
        const StaticResultContext *required;
        bool known;
        if (!static_closure_require(plan, entry->context, field->type, field->definition->site, &required, &known)) { return false; }
        ready &= known;
    }
    *output = ready;
    return true;
}
