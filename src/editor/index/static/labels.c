/** @file
 * @brief Retain actual named static binding origins from every accepted interface normalization context.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) distinguishes same-spelled formals in different actual constructors.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) keeps actual named input/output bindings separate from local receiver aliases.
 */
#include "../../analysis/internal.h"

/** Signature-only slots have no actual original declaration and are omitted,
 * never mapped back to a same-spelled local parameter. Every actual context
 * contributes its exact original StaticSymbol to the shared source spelling.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) reuses accepted binding evidence without another resolver.
 */
bool editor_index_static_labels(EditorAnalysis *analysis) {
    const BindingPlan *bindings = &analysis->frontend.specializations.bindings;
    for (const BindingApplication *application = bindings->first_application; application; application = application->next) {
        for (const BindingRequest *request = application->first; request; request = request->next) {
            if (request->direction != BINDING_STATIC_INPUT && request->direction != BINDING_STATIC_OUTPUT) { continue; }
            if (!request->named || request->omitted || !request->label.source || !request->slot || !request->slot->static_symbol) { continue; }
            if (!editor_index_add(analysis, (EditorOccurrence){.spelling=request->label,.static_symbol=request->slot->static_symbol})) { return false; }
        }
    }
    return true;
}
