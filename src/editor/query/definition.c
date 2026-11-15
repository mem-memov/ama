/** @file
 * @brief Project exact indexed byte ranges into borrowed checked definition/signature metadata.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) retains original byte locations.
 */
#include "../analysis/analysis.h"
/** Half-open spelling membership never matches separators, comments or an invented shorter name. */
bool editor_definition(const EditorAnalysis *analysis, const char *module, size_t offset, EditorDefinition *output) {
    if (!analysis->indexed || output == NULL) { return false; }
    const ModuleSource *source = NULL;
    if (module_catalog_find(&analysis->frontend.project.catalog, module, &source) != MODULE_OK
        || offset >= source_length(source->text)) { return false; }
    for (const EditorOccurrence *item = analysis->first; item != NULL; item = item->next) {
        if (item->spelling.source != source->text || offset < item->spelling.start || offset >= item->spelling.end) { continue; }
        SourceSpan definition = item->symbol != NULL ? item->symbol->name
            : item->type != NULL ? (SourceSpan){item->type->module->source->text,0,0} : (SourceSpan){0};
        if (definition.source == NULL) { return false; }
        *output = (EditorDefinition){item->symbol,item->type,definition};
        return true;
    }
    return false;
}
