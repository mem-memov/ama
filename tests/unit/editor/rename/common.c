/** @file
 * @brief Compose counted complete-workspace and rename owners for exact versioned edit checks.
 */
#ifndef AMA_RENAME_TEST_COMMON
#define AMA_RENAME_TEST_COMMON
#include "../common.c"
#include "editor/rename/rename.h"
static EditorRename rename_plan;
/** Construct the complete checked source domain before preparing any edit. */
static void setup_rename_roots(const ModuleRoot *roots, size_t count) {
    setup_editor();
    bool valid = editor_workspace_build(&analysis,roots,count,"/test/fixture/v1.0.0/src/main");
    if (!valid) { fprintf(stderr,"Rename workspace: %s\n",analysis.diagnostic.message); }
    assert(valid && analysis.workspace);
    editor_rename_init(&rename_plan,sources.allocator);
}
/** Use one explicit project root for fixtures without standard/native type requirements. */
static void setup_rename(void) {
    ModuleRoot root = {"generated/project",NULL,0,MODULE_ROOT_PROJECT};
    setup_rename_roots(&root,1);
}
/** Candidate/new-byte owners retire before the borrowed original workspace. */
static void destroy_rename(void) {
    editor_rename_destroy(&rename_plan);
    destroy_editor();
}
/** Select the exact occurrence at an unambiguous fixture needle in any active workspace module. */
static bool prepare(const char *module, const char *needle, const char *name) {
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&analysis.frontend.project.catalog,module,&source) == MODULE_OK);
    return editor_rename_prepare(&rename_plan,&analysis,module,point(source->text,needle),(const unsigned char *)name,strlen(name));
}
/** Find a full-file replacement by its exact logical identity rather than edited basename guessing. */
static inline const EditorRenameFile *changed_file(const char *module) {
    for (const EditorRenameFile *file = rename_plan.first; file != NULL; file = file->next) {
        if (strcmp(file->original->logical_path,module) == 0) { return file; }
    }
    return NULL;
}
/** Assertion diagnostics identify the exact compiler/identity gate that rejected a candidate. */
static inline bool validate(void) {
    bool valid = editor_rename_validate(&rename_plan);
    if (!valid) { fprintf(stderr,"Rename validation: %s\n",rename_plan.diagnostic.message); }
    return valid;
}
#endif
