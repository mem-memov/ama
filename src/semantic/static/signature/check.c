/**
 * @file
 * @brief Resolve available signature categories and retain explicit unresolved pattern obligations.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) fixes category from
 * declared metadata, never first actual values. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * gives output-phase expressions a closed producer Type, while selected aliases
 * require producer metadata. Applications await the application normalization
 * stage; their exact expressions remain in signature entries.
 */
#include "internal.h"

/** Known signature disagreement is a present error; projected/application
 * metadata remains a pending pattern for subsequent normalization/resolution.
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
    for (StaticModule *module = inventory->first; module != NULL; module = module->next) {
        for (StaticPattern *pattern = module->first_pattern; pattern != NULL; pattern = pattern->next) {
            StaticMetadata producer = static_expression_metadata(inventory, module, pattern->occurrence->expression);
            /* The application value may be closed; patterns constrain the head's
             * expected constructor signature instead of that value's category. */
            const StaticOccurrence *site = pattern->occurrence;
            if (site->symbol != NULL) { producer = (StaticMetadata){site->symbol->category, site->symbol->signature}; }
            else {
                const StaticModule *origin = static_module_find(inventory, site->origin);
                producer = origin->constructor_signature == NULL ? (StaticMetadata){STATIC_CATEGORY_TYPE, NULL}
                    : (StaticMetadata){STATIC_CATEGORY_TEMPLATE, origin->constructor_signature};
            }
            if (producer.category == STATIC_CATEGORY_DEFERRED) { pattern->pending = true; continue; }
            if (producer.category != STATIC_CATEGORY_TEMPLATE) {
                return static_invalid(inventory, site->site, "An input pattern requires a Template constructor", site->symbol);
            }
            StaticSignatureComparison result;
            if (!static_signature_compare(inventory, pattern->signature, producer.signature,
                pattern->output_phase ? STATIC_SIGNATURE_PROVIDED : STATIC_SIGNATURE_EXACT, &result)) { return false; }
            if (result == STATIC_SIGNATURE_DIFFERENT) {
                return static_invalid(inventory, site->site, "Input pattern disagrees with the declared constructor signature", site->symbol);
            }
            pattern->pending = result == STATIC_SIGNATURE_PENDING;
        }
    }
    return true;
}
