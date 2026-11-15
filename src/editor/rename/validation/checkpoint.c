/** @file
 * @brief Require exact original source inventory, physical identity and bytes before candidate editing.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) forbids stale inputs;
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06) uses full equality, never hash guesses.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"
/** Total original physical-path order supports exact logarithmic coordinate lookup. */
static int compare(const void *left, const void *right) {
    const EditorRenameSource *a = left, *b = right;
    return strcmp(source_path(a->original->text),source_path(b->original->text));
}
/** The fresh catalog must be a bijection of the old logical/physical records, including opaque drafts. */
bool editor_rename_checkpoint(EditorRename *plan) {
    ModuleCatalog *catalog = &plan->candidate.frontend.project.catalog;
    ModuleStatus built = module_catalog_build(catalog,plan->workspace->roots,plan->workspace->root_count);
    if (built != MODULE_OK) {
        bool adapted = diagnostic_from_module(&catalog->issue,&plan->candidate.frontend.project.catalog_related,&plan->diagnostic);
        (void)adapted;
        return false;
    }
    size_t count = 0, fresh_count = 0;
    for (const ModuleSource *source = plan->workspace->frontend.project.catalog.first; source != NULL; source = source->next) {
        if (count == SIZE_MAX/sizeof(EditorRenameSource)) { goto memory; }
        count++;
    }
    for (const ModuleSource *source = catalog->first; source != NULL; source = source->next) { fresh_count++; }
    if (fresh_count != count) { goto changed; }
    EditorRenameSource *pairs = source_arena_allocate(&plan->candidate.arena,count*sizeof(*pairs));
    if (pairs == NULL) { goto memory; }
    size_t index = 0;
    for (const ModuleSource *old = plan->workspace->frontend.project.catalog.first; old != NULL; old = old->next) {
        const ModuleSource *current = NULL;
        if (module_catalog_find(catalog,old->logical_path,&current) != MODULE_OK
            || strcmp(source_path(old->text),source_path(current->text)) != 0
            || old->device != current->device || old->inode != current->inode
            || source_length(old->text) != source_length(current->text)
            || memcmp(source_bytes(old->text),source_bytes(current->text),source_length(old->text)) != 0) { goto changed; }
        const EditorRenameFile *file = plan->first;
        while (file != NULL && file->original != old) { file = file->next; }
        pairs[index++] = (EditorRenameSource){old,current,file};
    }
    qsort(pairs,count,sizeof(*pairs),compare);
    plan->source_index = pairs;
    plan->source_count = count;
    return true;
changed:
    return editor_rename_fail(plan,DIAGNOSTIC_CHANGED_INPUT,"Workspace source inventory or bytes changed after rename preparation",plan->target->name);
memory:
    return editor_rename_fail(plan,DIAGNOSTIC_OUT_OF_MEMORY,"Rename source checkpoint allocation failed",plan->target->name);
}
