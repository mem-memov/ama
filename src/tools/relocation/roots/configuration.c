/** @file
 * @brief Copy explicit settings and project only physically carried roots.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves carried root kinds and accepts an explicit final destination root.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * forbids inferring a namespace family from a directory name.
 */
#include <stdint.h>
#include "../internal.h"

/** Deep-copy root-relative exclusion bytes; they remain relative to their own root after movement. */
static bool exclusions(RelocationPreview *preview, const ModuleRoot *input, ModuleRoot *output) {
    if ((input->excluded_count != 0 && input->excluded == NULL)
        || input->excluded_count > SIZE_MAX/sizeof(char *)) {
        return relocation_fail(preview,RELOCATION_INVALID_PATH,output->path,NULL);
    }
    if (input->excluded_count == 0) { return true; }
    const char **items = source_arena_allocate(&preview->arena,input->excluded_count*sizeof(*items));
    if (items == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,output->path,NULL); }
    for (size_t index = 0; index < input->excluded_count; index++) {
        if (input->excluded[index] == NULL) { return relocation_fail(preview,RELOCATION_INVALID_PATH,output->path,NULL); }
        items[index] = source_arena_copy(&preview->arena,input->excluded[index]);
        if (items[index] == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,output->path,NULL); }
    }
    output->excluded = items;
    output->excluded_count = input->excluded_count;
    return true;
}

/** Resolve only directory authority; module-name grammar is applied below these root boundaries. */
static bool copy_root(RelocationPreview *preview, const ModuleRoot *input, bool final, ModuleRoot *output) {
    if (input->kind != MODULE_ROOT_PROJECT && input->kind != MODULE_ROOT_STANDARD) {
        return relocation_fail(preview,RELOCATION_INVALID_PATH,NULL,NULL);
    }
    output->kind = input->kind;
    output->path = final ? relocation_final_path(preview,input->path) : relocation_path(preview,input->path,false);
    return output->path != NULL && exclusions(preview,input,output);
}

/** Allocate both complete arrays before publishing either, retaining failed prefixes in the preview arena. */
bool relocation_roots(RelocationPreview *preview, const RelocationRequest *request) {
    if ((request->root_count != 0 && request->roots == NULL)
        || request->root_count >= SIZE_MAX/sizeof(ModuleRoot)) {
        return relocation_fail(preview,RELOCATION_INVALID_PATH,NULL,NULL);
    }
    size_t after_count = request->root_count+(request->destination_root != NULL ? 1 : 0);
    ModuleRoot *before = source_arena_allocate(&preview->arena,request->root_count*sizeof(*before));
    ModuleRoot *after = source_arena_allocate(&preview->arena,after_count*sizeof(*after));
    if (before == NULL || after == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,NULL,NULL); }
    bool covered = false;
    for (size_t index = 0; index < request->root_count; index++) {
        before[index] = (ModuleRoot){0};
        if (!copy_root(preview,&request->roots[index],false,&before[index])) { return false; }
        after[index] = before[index];
        after[index].path = relocation_project(preview,before[index].path);
        if (after[index].path == NULL) { return false; }
    }
    if (request->destination_root != NULL) {
        after[request->root_count] = (ModuleRoot){0};
        if (!copy_root(preview,request->destination_root,true,&after[request->root_count])) { return false; }
    }
    for (size_t index = 0; index < after_count; index++) {
        if (relocation_contains(after[index].path,preview->destination)
            || relocation_contains(preview->destination,after[index].path)) { covered = true; }
    }
    if (!covered) { return relocation_fail(preview,RELOCATION_UNCOVERED,preview->destination,NULL); }
    preview->before_roots = before;
    preview->after_roots = after;
    preview->before_root_count = request->root_count;
    preview->after_root_count = after_count;
    return true;
}
