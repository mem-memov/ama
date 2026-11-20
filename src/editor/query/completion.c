/** @file
 * @brief Enumerate source declarations and static interfaces without selecting an arbitrary concrete specialization.
 * [NAME-06](../../../spec/language/declarations.md#name-06) delegates lexical/construction visibility to the compiler source inventory.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) gives graph parameters/results whole-module visibility without runtime slots.
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) keeps ordinary interface directions distinct.
 */
#include "../analysis/analysis.h"
#include "semantic/static/connections/internal.h"

/** Query one known declaration site to retain all actual typed projections.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) uses accepted source identities rather than a shadow symbol table.
 */
static bool visit_definition(const EditorAnalysis *analysis, const char *module, SourceSpan name,
    EditorSymbolVisitor visit, void *context, bool *stopped) {
    EditorDefinition value;
    if (!editor_definition(analysis, module, name.start, &value)) { return false; }
    *stopped = !visit(context, &value);
    return true;
}

/** Match one loaded original module, not the first runtime Type sharing its name.
 * [TPL-02](../../../spec/language/templates.md#tpl-02) retains open source definitions independently from their instances.
 */
static const StaticModule *source_module(const EditorAnalysis *analysis, const char *module) {
    if (!analysis || !analysis->indexed || !module) { return NULL; }
    const ProjectModule *source = project_find(&analysis->frontend.project, module);
    return source ? static_module_find(&analysis->frontend.static_inventory, source) : NULL;
}

bool editor_completions(const EditorAnalysis *analysis, const char *module, size_t offset,
    EditorSymbolVisitor visit, void *context) {
    const StaticModule *source = source_module(analysis, module);
    if (!source || !visit || offset > source_length(source->definition->source->text)) { return false; }
    SourceSpan site = {source->definition->source->text,offset,offset};
    bool stopped = false;
    for (const StaticRuntimeModule *runtime = analysis->frontend.specializations.connections.first_runtime; runtime; runtime = runtime->next) {
        if (runtime->source != source) { continue; }
        const StaticRuntimeField *initializing = static_runtime_initializing_at(runtime, offset);
        for (const StaticRuntimeField *field = runtime->first_field; field; field = field->next) {
            if (!static_runtime_available(runtime, field, site, initializing)) { continue; }
            if (!visit_definition(analysis, module, field->name, visit, context, &stopped)) { return false; }
            if (stopped) { return true; }
        }
    }
    for (const StaticSymbol *symbol = source->first; symbol; symbol = symbol->next) {
        if (!visit_definition(analysis, module, symbol->name, visit, context, &stopped)) { return false; }
        if (stopped) { return true; }
    }
    return true;
}

bool editor_signature(const EditorAnalysis *analysis, const char *module, SymbolRole role,
    EditorSymbolVisitor visit, void *context) {
    const StaticModule *source = source_module(analysis, module);
    if (!source || !visit || (role != SYMBOL_PARAMETER && role != SYMBOL_RESULT && role != SYMBOL_CONSTANT)) { return false; }
    bool stopped = false;
    if (source->definition->syntax.sections.kind == MODULE_STATIC) {
        for (const StaticSymbol *symbol = source->first; symbol; symbol = symbol->next) {
            if (role == SYMBOL_CONSTANT || (role == SYMBOL_PARAMETER) != (symbol->role == STATIC_PARAMETER)) { continue; }
            if (!visit_definition(analysis, module, symbol->name, visit, context, &stopped)) { return false; }
            if (stopped) { break; }
        }
        return true;
    }
    for (const StaticRuntimeModule *runtime = analysis->frontend.specializations.connections.first_runtime; runtime; runtime = runtime->next) {
        if (runtime->source != source) { continue; }
        for (const StaticRuntimeField *field = runtime->first_field; field; field = field->next) {
            if (field->scope || field->role != (role == SYMBOL_PARAMETER ? STATIC_FIELD_INPUT : role == SYMBOL_RESULT ? STATIC_FIELD_RESULT : STATIC_FIELD_CONSTANT)) { continue; }
            if (!visit_definition(analysis, module, field->name, visit, context, &stopped)) { return false; }
            if (stopped) { return true; }
        }
    }
    return true;
}
