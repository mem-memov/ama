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
    static_resolve_metadata(inventory);
    if (!static_signatures_acyclic(inventory)) { return false; }
    for (const StaticModule *module = inventory->first; module != NULL; module = module->next) {
        for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
            if (site->receiver != NULL) { continue; }
            bool type = site->symbol != NULL ? site->symbol->category == STATIC_CATEGORY_TYPE
                : static_module_find(inventory, site->origin)->constructor_signature == NULL;
            if (!type) { continue; }
            for (const AstStaticBinding *binding = site->expression->first; binding != NULL; binding = binding->next) {
                if (binding->kind == AST_STATIC_INPUT) {
                    return static_invalid(inventory, binding->marker, "A Type is not an applied constructor", site->symbol);
                }
            }
        }
    }
    return true;
}
