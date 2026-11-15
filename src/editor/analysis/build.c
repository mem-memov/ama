/** @file
 * @brief Reuse one production project snapshot for checked metadata and exact draft inspection.
 * [SYN-04](../../../spec/language/source-and-names.md#syn-04) leaves drafts opaque;
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) forbids a second type system.
 */
#include <assert.h>
#include "internal.h"
/** Check exactly once. Reparse only the retained selected source for draft classification;
 * no second filesystem load can silently replace the frontend's immutable source bytes.
 */
bool editor_analysis_build(EditorAnalysis *analysis, const ModuleRoot *roots, size_t count, const char *entry) {
    assert(!analysis->attempted);
    analysis->attempted = true;
    bool checked = compiler_frontend_check(&analysis->frontend, roots, count, entry);
    analysis->diagnostic = analysis->frontend.error;
    if (!analysis->frontend.project.catalog.built
        || module_catalog_find(&analysis->frontend.project.catalog, entry, &analysis->selected) != MODULE_OK) {
        return false;
    }
    if (checked) {
        if (!editor_index_build(analysis)) { return false; }
        analysis->indexed = true;
        analysis->state = EDITOR_COMPLETE;
        return true;
    }
    if (analysis->diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY) { return false; }
    ParseStatus parsed = parser_module_parse(&analysis->inspection, analysis->selected->text);
    if (parsed != PARSE_OK) {
        analysis->diagnostic = analysis->inspection.error.diagnostic;
        analysis->state = EDITOR_INVALID;
        return analysis->diagnostic.code != DIAGNOSTIC_OUT_OF_MEMORY;
    }
    analysis->state = analysis->inspection.sections.kind == MODULE_DRAFT ? EDITOR_DRAFT : EDITOR_INVALID;
    if (analysis->state == EDITOR_DRAFT) { analysis->diagnostic = (Diagnostic){0}; }
    return true;
}
