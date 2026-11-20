/** @file
 * @brief Resolve original source definitions while retaining all actual concrete targets of a source spelling.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) retains exact original byte locations.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) forbids selecting a guessed concrete context.
 * [NAME-06](../../../spec/language/declarations.md#name-06) fixes source declaration identity.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) gives static graphs source definitions without runtime Types.
 */
#include "../analysis/analysis.h"

bool editor_binding_targets(const EditorAnalysis *analysis, const char *module, size_t offset,
    const EditorOccurrence **output) {
    if (!analysis || !module || !analysis->indexed || output == NULL) { return false; }
    const ModuleSource *source = NULL;
    if (module_catalog_find(&analysis->frontend.project.catalog, module, &source) != MODULE_OK
        || offset >= source_length(source->text)) { return false; }
    for (const EditorOccurrence *item = analysis->first; item; item = item->next) {
        if (item->spelling.source == source->text && offset >= item->spelling.start && offset < item->spelling.end) {
            *output = item;
            return true;
        }
    }
    return false;
}

/** Source identities are independent of concrete runtime variants; module
 * definitions use their real source start rather than a generated Type file.
 * [NAME-06](../../../spec/language/declarations.md#name-06) preserves lexical declarations.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps static and runtime domains separate.
 */
static SourceSpan definition(const EditorOccurrence *target) {
    if (target->static_symbol) { return target->static_symbol->site; }
    if (target->original) { return target->original->name; }
    if (target->symbol) { return target->symbol->name; }
    if (target->module) { return (SourceSpan){target->module->source->text,0,0}; }
    return (SourceSpan){0};
}

/** Absence supplies no concrete evidence; disagreement permanently suppresses
 * a singular metadata field while the alternatives retain both actual values.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) prohibits choosing the first inferred Type or origin.
 */
static const void *unique(const void *current, const void *other, bool *conflict) {
    if (*conflict || !other) { return current; }
    if (!current || current == other) { return other; }
    *conflict = true;
    return NULL;
}

bool editor_definition(const EditorAnalysis *analysis, const char *module, size_t offset, EditorDefinition *output) {
    const EditorOccurrence *targets;
    if (!output || !editor_binding_targets(analysis, module, offset, &targets)) { return false; }
    EditorDefinition value = {.targets=targets};
    bool conflicts[6] = {false};
    for (const EditorOccurrence *target = targets; target; target = target->alternatives) {
        SourceSpan site = definition(target);
        if (!site.source) { return false; }
        if (!value.definition.source) { value.definition = site; }
        if (value.definition.source != site.source || value.definition.start != site.start || value.definition.end != site.end) { return false; }
        value.symbol = unique(value.symbol, target->symbol, &conflicts[0]);
        value.type = unique(value.type, target->type, &conflicts[1]);
        value.static_symbol = unique(value.static_symbol, target->static_symbol, &conflicts[2]);
        value.module = unique(value.module, target->module, &conflicts[3]);
        value.original = unique(value.original, target->original, &conflicts[4]);
        value.runtime = unique(value.runtime, target->runtime, &conflicts[5]);
    }
    *output = value;
    return true;
}
