/**
 * @file
 * @brief Propagate independently known static categories to a finite monotone fixed point.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) fixes categories
 * from declared metadata rather than actual processing order.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) leaves selected
 * aliases pending until producer metadata is known. Input applications retain
 * their original expressions for the subsequent normalization stage.
 */
#include "internal.h"

/** Locate the already bound head of this exact AST expression owner.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static const StaticOccurrence *head(const StaticModule *module, const AstTypeExpression *expression) {
    for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver == NULL && site->expression == expression) { return site; }
    }
    return NULL;
}

/** Read independently available metadata only. Input applications require
 * normalization before their residual category can be published; an output
 * phase promises a closed producer Type but does not execute it.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
StaticMetadata static_expression_metadata(const StaticInventory *inventory, const StaticModule *module,
    const AstTypeExpression *expression) {
    bool inputs = false;
    for (const AstStaticBinding *binding = expression->first; binding != NULL; binding = binding->next) {
        if (binding->kind == AST_STATIC_OUTPUT) { return (StaticMetadata){STATIC_CATEGORY_TYPE, NULL}; }
        inputs = true;
    }
    if (inputs) { return (StaticMetadata){0}; }
    const StaticOccurrence *site = head(module, expression);
    if (site->symbol != NULL) { return (StaticMetadata){site->symbol->category, site->symbol->signature}; }
    const StaticModule *origin = static_module_find(inventory, site->origin);
    return origin->constructor_signature == NULL ? (StaticMetadata){STATIC_CATEGORY_TYPE, NULL}
        : (StaticMetadata){STATIC_CATEGORY_TEMPLATE, origin->constructor_signature};
}

/** Publish one previously deferred metadata pair at most once. Information is
 * monotone: every true result reduces the finite number of unresolved slots.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool assign(StaticCategory *category, StaticSignature **signature, StaticMetadata value) {
    if (*category != STATIC_CATEGORY_DEFERRED || value.category == STATIC_CATEGORY_DEFERRED) { return false; }
    *category = value.category;
    *signature = value.signature;
    return true;
}

/** Propagate known local result and constructor-slot categories to a finite
 * fixed point. Producer-result aliases and applied residuals remain recorded.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
void static_resolve_metadata(StaticInventory *inventory) {
    bool changed;
    do {
        changed = false;
        for (StaticModule *module = inventory->first; module != NULL; module = module->next) {
            for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
                if (symbol->role == STATIC_RESULT) {
                    changed |= assign(&symbol->category, &symbol->signature,
                        static_expression_metadata(inventory, module, symbol->declaration->as.static_result.expression));
                }
            }
            if (module->constructor_signature == NULL) { continue; }
            for (StaticSignatureEntry *entry = module->constructor_signature->first; entry != NULL; entry = entry->next) {
                const StaticSymbol *symbol = static_symbol_find(module, entry->name, entry->name.start);
                changed |= assign(&entry->category, &entry->signature,
                    (StaticMetadata){symbol->category, symbol->signature});
            }
        }
    } while (changed);
}
