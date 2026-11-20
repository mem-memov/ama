/** @file
 * @brief Validate exact in-memory source replacements through the ordinary complete workspace pipeline.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) reuses semantic rules;
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) rejects changed native dependencies.
 */
#include "../internal.h"
/** Native implementation/header/protocol bytes are not an Ama-name edit and must remain exactly the captured inputs. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
static bool native_input(EditorRename *plan, const char *path) {
    if (path == NULL) { return true; }
    bool changed = false;
    if (!project_inputs_changed(&plan->candidate.frontend.inputs,PROJECT_INPUT_FILE,path,&changed) || changed) {
        (void)editor_rename_fail(plan,DIAGNOSTIC_CHANGED_INPUT,"Native input changed after rename preparation",plan->target.definition);
        plan->diagnostic.subject = path;
        return false;
    }
    return true;
}
/** Publish no candidate semantic metadata until every active importer resolves and all native input versions agree. Component contract: [COMP-02](../../../../spec/implementation/compiler.md#comp-02), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
bool editor_rename_candidate(EditorRename *plan) {
    ModuleCatalog *catalog = &plan->candidate.frontend.project.catalog;
    for (const EditorRenameFile *file = plan->first; file != NULL; file = file->next) {
        const ModuleSource *source = NULL;
        if (module_catalog_find(catalog,file->original->logical_path,&source) != MODULE_OK) {
            return editor_rename_fail(plan,DIAGNOSTIC_CHANGED_INPUT,"Rename source disappeared from its candidate",plan->target.definition);
        }
        ModuleStatus status = module_catalog_override(catalog,source,source_bytes(file->replacement),source_length(file->replacement));
        if (status != MODULE_OK) {
            bool adapted = diagnostic_from_module(&catalog->issue,&plan->candidate.frontend.project.catalog_related,&plan->diagnostic);
            (void)adapted;
            return false;
        }
    }
    (void)module_catalog_find(catalog,plan->workspace->entry,&plan->candidate.selected);
    if (!editor_workspace_finish(&plan->candidate)) {
        plan->diagnostic = plan->candidate.diagnostic;
        if (plan->diagnostic.message == NULL) {
            return editor_rename_fail(plan,DIAGNOSTIC_OUT_OF_MEMORY,"Rename candidate analysis allocation failed",plan->target.definition);
        }
        return false;
    }
    if (project_inputs_compare(&plan->candidate.frontend.inputs,&plan->workspace->frontend.inputs) != SOURCE_OK) {
        return editor_rename_fail(plan,DIAGNOSTIC_RENAME_CONFLICT,"Rename input snapshots could not be compared",plan->target.definition);
    }
    for (const ModuleSource *old = plan->workspace->frontend.project.catalog.first; old != NULL; old = old->next) {
        if (old->native_loaded && (!native_input(plan,old->native_c) || !native_input(plan,old->native_header)
            || !native_input(plan,old->native_abi == NULL ? NULL : source_path(old->native_abi)))) { return false; }
    }
    return true;
}
