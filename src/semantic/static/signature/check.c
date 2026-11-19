/**
 * @file
 * @brief Check known constructor metadata without expected-signature source declarations.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) fixes category from
 * declared metadata, never first actual values. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * requires producer metadata before selected outputs can be concrete. Applications
 * await normalization; known bare Type heads cannot accept static arguments.
 */
#include "internal.h"

/** Known signature disagreement is a present error; projected/application
 * metadata remains pending for subsequent normalization/resolution.
 * No signature comparison may traverse known cyclic metadata.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_check_signatures(StaticInventory *inventory) {
    if (inventory->status != STATIC_OK) { return false; }
    static_prepare_usage_groups(inventory);
    static_resolve_metadata(inventory);
    if (!static_signatures_acyclic(inventory)) { return false; }
    for (const StaticModule *module = inventory->first; module != NULL; module = module->next) {
        for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
            if (site->receiver != NULL || site->symbol != NULL) { continue; }
            if (site->origin->syntax.sections.kind == MODULE_STATIC) { continue; }
            StaticMetadata actual = static_head_metadata(inventory, module, site->expression);
            StaticUsageResult checked = static_usage_expression(inventory, module,
                site->expression, actual.category, actual.signature, NULL);
            if (checked.status == STATIC_USAGE_CONFLICT) {
                return static_invalid(inventory, checked.site, checked.message, NULL);
            }
        }
        for (const StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            StaticUsageResult checked = static_usage_check(inventory, symbol, symbol->category, symbol->signature);
            if (checked.status == STATIC_USAGE_CONFLICT) {
                return static_invalid(inventory, checked.site, checked.message, symbol);
            }
        }
    }
    return true;
}
