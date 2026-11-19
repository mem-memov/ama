/**
 * @file
 * @brief Classify ordinary Type formals or graph set-complement inputs after every public binder.
 *
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates public graph
 * binders from ordinary Type interfaces. [TPL-02](../../../../spec/language/templates.md#tpl-02)
 * collects ordinary Type names without using actual values.
 */
#include "sites.h"

/** All public pair/receipt names are already fixed. A graph's remaining local
 * source heads are free inputs with deferred actual metadata; ordinary modules
 * introduce Type only through input/result Type lines. Foreign labels add none.
 * Source order fixes first introductions; solving never changes binder roles.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_classify_names(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    bool graph = module->definition->syntax.sections.kind == MODULE_STATIC;
    for (StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver != NULL || site->expression->kind != AST_TYPE_PLACEHOLDER) { continue; }
        if (static_symbol_find(module, site->site) != NULL) { continue; }
        StaticCategory category = graph ? STATIC_CATEGORY_DEFERRED : static_introduction_candidate(site);
        if (!graph && category == STATIC_CATEGORY_DEFERRED) { continue; }
        StaticSymbol *symbol = static_symbol_add(inventory, module, site->site, site->site,
            STATIC_PARAMETER, category);
        if (symbol == NULL) { return false; }
        symbol->declaration = site->statement;
    }
    static_order_symbols(module, STATIC_PARAMETER);
    return true;
}
