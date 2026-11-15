/** @file
 * @brief Analyze every configured active source through one checked compiler pipeline before identity-aware edits.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) prevents unused importers from escaping validation;
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) reuses compiler binding rules.
 */
#include <assert.h>
#include "internal.h"
/** Complete only after all active source modules and exact settings have passed normal semantic/planning checks. */
bool editor_workspace_finish(EditorAnalysis *analysis) {
    const char *const *required = NULL;
    size_t count = 0;
    if (!editor_workspace_active(analysis,&required,&count)) { return false; }
    if (!compiler_frontend_check_catalog(&analysis->frontend,analysis->roots,analysis->root_count,
        analysis->entry,required,count)) {
        analysis->diagnostic = analysis->frontend.error;
        return false;
    }
    if (!editor_index_build(analysis)) { return false; }
    analysis->indexed = analysis->workspace = true;
    analysis->state = EDITOR_COMPLETE;
    return true;
}
/** Own one fresh complete catalog, classify active inputs and retain all failure diagnostics until destruction. */
bool editor_workspace_build(EditorAnalysis *analysis, const ModuleRoot *roots, size_t count, const char *entry) {
    assert(!analysis->attempted);
    analysis->attempted = true;
    bool valid = editor_workspace_configuration(analysis,roots,count,entry);
    if (valid) {
        ModuleStatus status = module_catalog_build(&analysis->frontend.project.catalog,analysis->roots,analysis->root_count);
        if (status != MODULE_OK) {
            valid = false;
            bool adapted = diagnostic_from_module(&analysis->frontend.project.catalog.issue,
                &analysis->frontend.project.catalog_related,&analysis->diagnostic);
            assert(adapted);
            (void)adapted;
        }
    }
    if (valid) {
        (void)module_catalog_find(&analysis->frontend.project.catalog,analysis->entry,&analysis->selected);
        valid = editor_workspace_finish(analysis);
    }
    if (!valid && analysis->diagnostic.message == NULL) {
        analysis->diagnostic = (Diagnostic){.code=DIAGNOSTIC_OUT_OF_MEMORY,.message="Workspace analysis allocation failed"};
    }
    if (!valid && analysis->diagnostic.code != DIAGNOSTIC_OUT_OF_MEMORY) { analysis->state = EDITOR_INVALID; }
    return valid;
}
