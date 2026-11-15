/** @file
 * @brief Expose an exact read-only filesystem view after P without creating a staged directory.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * maps D descendants back to S for reads and removes S from its old parent.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

/** P is injective because D was absent and disjoint from S; outside both subtrees paths stay unchanged. */
const char *relocation_original_path(RelocationPreview *preview, const char *after) {
    if (relocation_contains(preview->destination,after)) {
        const char *before = source_arena_join(&preview->arena,preview->source,"",after+strlen(preview->destination));
        if (before == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,after,NULL); }
        return before;
    }
    return relocation_contains(preview->source,after) ? NULL : after;
}

/** Original parent/name lookup preserves exact spelling even when D and S have different basenames. */
PathProbe relocation_probe(RelocationPreview *preview, const char *after, const char **physical, struct stat *metadata) {
    *physical = relocation_original_path(preview,after);
    if (*physical == NULL) { return preview->status == RELOCATION_OK ? PATH_MISSING : PATH_IO_ERROR; }
    if (strcmp(*physical,"/") == 0) { return lstat("/",metadata) == 0 ? PATH_FOUND : PATH_IO_ERROR; }
    char *parent = source_arena_copy(&preview->arena,*physical);
    if (parent == NULL) { relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,after,NULL); return PATH_IO_ERROR; }
    char *slash = strrchr(parent,'/');
    const char *name = *physical+(slash-parent)+1;
    if (slash == parent) { slash[1] = 0; } else { *slash = 0; }
    return path_exact_child(parent,name,*physical,metadata);
}

/** Locale-independent order matches the ordinary catalog's byte order. */
static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left,*(const char *const *)right);
}

/** Filter only exact structural paths; an unrelated name sharing a textual prefix remains untouched. */
bool relocation_view_entries(RelocationPreview *preview, const char *after, const char ***output, size_t *output_count) {
    const char *before = relocation_original_path(preview,after);
    if (before == NULL) { return relocation_fail(preview,RELOCATION_INVALID_PATH,after,NULL); }
    const char **names = NULL;
    size_t count = 0;
    if (!relocation_entries(preview,before,&names,&count)) { return false; }
    if (count >= SIZE_MAX/sizeof(char *)) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,after,NULL); }
    const char **selected = source_arena_allocate(&preview->arena,(count+1)*sizeof(*selected));
    if (selected == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,after,NULL); }
    size_t retained = 0;
    for (size_t index = 0; index < count; index++) {
        const char *full = source_arena_join(&preview->arena,after,strcmp(after,"/") == 0 ? "" : "/",names[index]);
        if (full == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,after,NULL); }
        if (strcmp(full,preview->source) != 0) { selected[retained++] = names[index]; }
    }
    const char *last = strrchr(preview->destination,'/');
    size_t parent_length = last == preview->destination ? 1 : (size_t)(last-preview->destination);
    if (strlen(after) == parent_length && strncmp(after,preview->destination,parent_length) == 0) {
        for (size_t index = 0; index < retained; index++) {
            if (strcmp(selected[index],last+1) == 0) { return relocation_fail(preview,RELOCATION_DESTINATION_EXISTS,preview->destination,NULL); }
        }
        selected[retained++] = last+1;
    }
    qsort(selected,retained,sizeof(*selected),compare_names);
    *output = selected;
    *output_count = retained;
    return true;
}
