/** @file
 * @brief Index original static definitions, module heads and exact known foreign labels from the accepted compiler inventory.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) distinguishes graph producers, exports and returned Types.
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) scopes parameters to original definitions.
 */
#include "../../analysis/internal.h"
#include "../../query/context.h"

/** A written module path identifies its actual original module. Only a
 * zero-parameter ordinary definition has an unambiguous concrete runtime Type.
 * [TPL-13](../../../../spec/language/templates.md#tpl-13) gives static modules no runtime instance.
 */
static bool module(EditorAnalysis *analysis, SourceSpan site, const ProjectModule *origin) {
    const StaticModule *definition = static_module_find(&analysis->frontend.static_inventory, origin);
    const SemanticType *type = definition->parameter_count == 0 && origin->syntax.sections.kind != MODULE_STATIC
        ? editor_query_type(analysis, origin->source->logical_path) : NULL;
    return editor_index_add(analysis, (EditorOccurrence){.spelling=site,.module=origin,.type=type});
}

/** Exact foreign labels belong to the written constructor/module interface,
 * never to a same-spelled local free parameter.
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) preserves scoped formal identity.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) makes static transfers named.
 */
static bool labels(EditorAnalysis *analysis, const StaticOccurrence *site) {
    const StaticModule *target = static_module_find(&analysis->frontend.static_inventory, site->origin);
    for (const AstStaticBinding *binding = site->expression->first; binding; binding = binding->next) {
        const StaticSymbol *symbol = static_symbol_find(target, binding->label);
        if (!symbol || !editor_index_add(analysis, (EditorOccurrence){.spelling=binding->label,.static_symbol=symbol})) { return false; }
    }
    if (site->expression->selected_export.source) {
        const StaticSymbol *symbol = static_symbol_find(target, site->expression->selected_export);
        if (!symbol || !editor_index_add(analysis, (EditorOccurrence){.spelling=site->expression->selected_export,.static_symbol=symbol})) { return false; }
    }
    return true;
}

/** Retain source declarations and resolved written references from the checked
 * inventory. First failure leaves owned records without publishing an index.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) supplies graph identity; no Type inference occurs here.
 */
bool editor_index_static(EditorAnalysis *analysis) {
    for (const StaticModule *owner = analysis->frontend.static_inventory.first; owner; owner = owner->next) {
        for (const StaticSymbol *symbol = owner->first; symbol; symbol = symbol->next) {
            if (!editor_index_add(analysis, (EditorOccurrence){.spelling=symbol->name,.static_symbol=symbol,.declaration=true})) { return false; }
        }
        for (const StaticOccurrence *site = owner->first_occurrence; site; site = site->next) {
            if (site->symbol && !editor_index_add(analysis, (EditorOccurrence){.spelling=site->site,.static_symbol=site->symbol})) { return false; }
            if (site->origin && (!module(analysis, site->site, site->origin) || !labels(analysis, site))) { return false; }
        }
        for (const StaticConstantOccurrence *site = owner->first_constant_source; site; site = site->next) {
            if (!module(analysis, site->source->module_path, site->origin)) { return false; }
        }
    }
    return true;
}
