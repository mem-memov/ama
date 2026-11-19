/**
 * @file
 * @brief Classify ordinary interface Type parameters after public-pair collection.
 *
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates public graph
 * binders from ordinary Type interfaces. [TPL-02](../../../../spec/language/templates.md#tpl-02)
 * collects ordinary Type names without using actual values.
 */
#include <stdint.h>
#include "sites.h"

/** Compute interface ordinals by complete introduction positions. Public
 * graph definitions occupy no ordinary formal position.
 * Counting predecessors avoids changing ownership links or source identities.
 * Component contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
static void ordinals(StaticModule *module) {
    for (StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role != STATIC_PARAMETER) { continue; }
        size_t ordinal = 0;
        for (const StaticSymbol *other = module->first; other != NULL; other = other->next) {
            if (other->role == STATIC_PARAMETER && other->site.start < symbol->site.start) { ordinal++; }
        }
        symbol->ordinal = ordinal;
    }
}

/** Sorted occurrences expose exactly the first eligible candidate/receiver for
 * every remaining complete name. Explicit names are already present and win
 * even before their textual declaration. Later receivers reuse those identities.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_classify_names(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    for (StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver == NULL && site->expression->kind != AST_TYPE_PLACEHOLDER) { continue; }
        if (static_symbol_find(module, site->site, SIZE_MAX) != NULL) { continue; }
        StaticCategory category = static_introduction_candidate(site);
        if (site->receiver == NULL && category == STATIC_CATEGORY_DEFERRED) { continue; }
        SourceSpan introduction = site->site;
        StaticSymbol *symbol = static_symbol_add(inventory, module, site->site, introduction,
            site->receiver != NULL ? STATIC_ALIAS : STATIC_PARAMETER, category);
        if (symbol == NULL) { return false; }
        symbol->declaration = site->statement;
        symbol->receiver = site->receiver;
    }
    ordinals(module);
    return true;
}
