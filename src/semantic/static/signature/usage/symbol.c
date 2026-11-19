/** @file
 * @brief Conjoin every immutable use requirement of one scoped graph name.
 * [TPL-07](../../../../../spec/language/templates.md#tpl-07) requires all uses,
 * preserving a forwarded-only actual without inventing a closed signature.
 */
#include "../internal.h"

StaticUsageResult static_usage_check(const StaticInventory *inventory, const StaticSymbol *symbol,
    StaticCategory category, const StaticSignature *signature) {
    StaticUsageResult result = {STATIC_USAGE_SATISFIED, {0}, NULL};
    for (const StaticOccurrence *use = symbol->owner->first_occurrence; use != NULL; use = use->next) {
        if (use->receiver != NULL || use->symbol == NULL
            || use->symbol->usage_group != symbol->usage_group) { continue; }
        StaticUsageResult checked = static_usage_expression(inventory, symbol->owner,
            use->expression, category, signature, symbol);
        if (checked.status == STATIC_USAGE_CONFLICT) { return checked; }
        if (checked.status == STATIC_USAGE_PENDING) { result = checked; }
    }
    return result;
}
