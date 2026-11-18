/**
 * @file
 * @brief Classify interface parameters and received aliases before binding local uses.
 *
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) gives the first
 * eligible candidate/receiver lexical precedence, after whole-module explicit
 * declarations. [TPL-02](../../../../spec/language/templates.md#tpl-02) then
 * collects whole-interface Template categories without using actual values.
 */
#include <stdint.h>
#include "sites.h"

/** Choose a Template introduction for an already classified implicit parameter,
 * if any. Earlier bare occurrences are forward uses of that Template rather
 * than independent Type declarations; its signature comes from the pattern.
 * Component contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
static const StaticOccurrence *template_site(const StaticModule *module, SourceSpan name) {
    for (const StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (static_introduction_candidate(site) == STATIC_CATEGORY_TEMPLATE
            && static_same_name(site->site, name)) { return site; }
    }
    return NULL;
}

/** Compute interface ordinals by complete introduction positions. Explicit
 * declarations keep their declaration positions; aliases/results occupy none.
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
        const StaticOccurrence *pattern = site->receiver == NULL ? template_site(module, site->site) : NULL;
        if (pattern != NULL) { category = STATIC_CATEGORY_TEMPLATE; }
        SourceSpan introduction = pattern != NULL ? pattern->site : site->site;
        StaticSymbol *symbol = static_symbol_add(inventory, module, site->site, introduction,
            site->receiver != NULL ? STATIC_ALIAS : STATIC_PARAMETER, category);
        if (symbol == NULL) { return false; }
        symbol->declaration = (pattern != NULL ? pattern : site)->statement;
        symbol->receiver = site->receiver;
    }
    ordinals(module);
    return true;
}
