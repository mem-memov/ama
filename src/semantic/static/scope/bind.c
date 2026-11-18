/**
 * @file
 * @brief Bind local static occurrences and exact loaded path requests without textual substitution.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) forbids undeclared
 * body parameters. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * prevents a later alias from repairing an earlier source. Concrete paths reuse
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02)'s loaded identity.
 */
#include "../internal.h"

/** Dependency collection preserves every original request span; equal basename
 * or normalized spelling is not sufficient to bind an origin.
 * Component contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
static const ProjectModule *origin(const StaticModule *module, SourceSpan site) {
    for (const ProjectDependency *edge = module->definition->first; edge != NULL; edge = edge->next) {
        if (!edge->implicit && edge->requirement.source == site.source
            && edge->requirement.start == site.start && edge->requirement.end == site.end) { return edge->target; }
    }
    return NULL;
}

/** Scan original lexical order, keeping foreign labels outside local lookup.
 * Inputs and source heads precede their own receivers, so self-sourcing a new
 * alias fails even though its later receiver already has an inventory record.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_bind_occurrences(StaticInventory *inventory, StaticModule *module) {
    if (inventory->status != STATIC_OK) { return false; }
    for (StaticOccurrence *site = module->first_occurrence; site != NULL; site = site->next) {
        if (site->receiver == NULL && site->expression->kind == AST_TYPE_MODULE_PATH) {
            site->origin = origin(module, site->site);
            if (site->origin == NULL) {
                return static_invalid(inventory, site->site, "Static path has no loaded dependency identity", NULL);
            }
        } else {
            site->symbol = static_symbol_find(module, site->site, site->site.start);
            if (site->symbol == NULL) {
                return static_invalid(inventory, site->site, "Static name is not visible at this source position", NULL);
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
    return true;
}
