/**
 * @file
 * @brief Bind local static occurrences and exact loaded path requests without textual substitution.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) forbids undeclared
 * body parameters. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * gives collected public definitions whole-graph visibility. Concrete paths reuse
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02)'s loaded identity.
 */
#include "../internal.h"

/** Scan original lexical order, keeping foreign labels outside local lookup.
 * All graph binders are collected before this walk; a forward use resolves to
 * its exact definition. Static-definition cycles are a later distinct check.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_bind_occurrences(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    for (StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver == NULL && site->expression->kind == AST_TYPE_MODULE_PATH) {
            site->origin = static_loaded_origin(module, site->site);
            if (site->origin == NULL) {
                return static_invalid(inventory, site->site, "Static path has no loaded dependency identity", NULL);
            }
        } else {
            site->symbol = static_symbol_find(module, site->site);
            if (site->symbol == NULL) {
                return static_invalid(inventory, site->site, "Static name has no declaration in this module", NULL);
            }
            if (site->receiver == NULL && site->symbol->category == STATIC_CATEGORY_TYPE) {
                for (const AstStaticBinding *binding = site->expression->first; binding != NULL; binding = binding->next) {
                    if (binding->kind == AST_STATIC_INPUT) {
                        return static_invalid(inventory, binding->marker, "A Type parameter is not an applied constructor", site->symbol);
                    }
                }
            }
        }
    }
    for (StaticConstantOccurrence *site = module->first_constant_source; site; site = site->next) {
        site->origin = static_loaded_origin(module, site->source->module_path);
        if (site->origin == NULL) {
            return static_invalid(inventory, site->source->module_path,
                "Qualified constant has no loaded dependency identity", NULL);
        }
    }
    return true;
}
