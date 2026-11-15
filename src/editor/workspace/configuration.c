/** @file
 * @brief Own exact configured root kinds, paths and exclusions for reproducible rename validation.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) makes declared root kind authoritative;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) forbids transient option borrows.
 */
#include <stdint.h>
#include "internal.h"
/** Copy checked finite settings without normalizing names, inferring kinds or retaining caller storage. */
bool editor_workspace_configuration(EditorAnalysis *analysis, const ModuleRoot *roots, size_t count, const char *entry) {
    if (!module_path_valid(entry) || (count != 0 && roots == NULL)) {
        analysis->diagnostic = (Diagnostic){.code=DIAGNOSTIC_INVALID_PATH,.message="Workspace roots or entry are invalid"};
        return false;
    }
    if (count > SIZE_MAX/sizeof(ModuleRoot)) { return false; }
    ModuleRoot *copy = source_arena_allocate(&analysis->arena,count*sizeof(*copy));
    if (copy == NULL) { return false; }
    for (size_t index = 0; index < count; index++) {
        if (roots[index].path == NULL || (roots[index].excluded_count != 0 && roots[index].excluded == NULL)) {
            analysis->diagnostic = (Diagnostic){.code=DIAGNOSTIC_INVALID_PATH,.message="Workspace root configuration is incomplete"};
            return false;
        }
        copy[index] = roots[index];
        copy[index].path = source_arena_copy(&analysis->arena,roots[index].path);
        size_t excluded_count = roots[index].excluded_count;
        if (excluded_count > SIZE_MAX/sizeof(char *)) { return false; }
        const char **excluded = source_arena_allocate(&analysis->arena,excluded_count*sizeof(*excluded));
        if (copy[index].path == NULL || excluded == NULL) { return false; }
        for (size_t item = 0; item < excluded_count; item++) {
            if (roots[index].excluded[item] == NULL) {
                analysis->diagnostic = (Diagnostic){.code=DIAGNOSTIC_INVALID_PATH,.message="Workspace exclusion is absent"};
                return false;
            }
            excluded[item] = source_arena_copy(&analysis->arena,roots[index].excluded[item]);
            if (excluded[item] == NULL) { return false; }
        }
        copy[index].excluded = excluded;
    }
    analysis->entry = source_arena_copy(&analysis->arena,entry);
    if (analysis->entry == NULL) { return false; }
    analysis->roots = copy;
    analysis->root_count = count;
    return true;
}
