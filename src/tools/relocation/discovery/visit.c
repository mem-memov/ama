/** @file
 * @brief Apply production Unicode candidate grammar and structural plus-directory rules to final placement.
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) supplies exact
 * complete names; [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * strips only structural directory plus and requires its regular sibling anchor.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

/** Exclusions match exact root-relative paths and descendants, never shared textual prefixes. */
static bool excluded(const ModuleRoot *root, const char *relative) {
    for (size_t index = 0; index < root->excluded_count; index++) {
        size_t length = strlen(root->excluded[index]);
        if (strncmp(relative,root->excluded[index],length) == 0 && (relative[length] == 0 || relative[length] == '/')) { return true; }
    }
    return false;
}

/** A moved plus directory receives no implicit sibling move; check its exact final regular anchor. */
static bool anchor(RelocationPreview *preview, const char *parent, const char *base) {
    const char *path = source_arena_join(&preview->arena,parent,"/",base);
    if (path == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,parent,NULL); }
    const char *physical = NULL;
    struct stat metadata;
    PathProbe probe = relocation_probe(preview,path,&physical,&metadata);
    if (probe == PATH_FOUND && S_ISREG(metadata.st_mode)) { return true; }
    return relocation_fail(preview,probe == PATH_IO_ERROR ? RELOCATION_IO_ERROR
        : probe == PATH_UNSUPPORTED ? RELOCATION_UNSUPPORTED_PATH : RELOCATION_ORPHAN,path,NULL);
}

/** Invalid candidate spellings remain opaque accompanying entries, matching ordinary source discovery. */
bool relocation_visit(RelocationWalk *walk, const RelocationDirectory *directory, const char *name) {
    RelocationPreview *preview = walk->preview;
    const char *relative = source_arena_join(&preview->arena,directory->relative,*directory->relative == 0 ? "" : "/",name);
    const char *path = source_arena_join(&preview->arena,directory->path,strcmp(directory->path,"/") == 0 ? "" : "/",name);
    if (relative == NULL || path == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,directory->path,NULL); }
    if (excluded(walk->root,relative)) { return true; }
    size_t length = strlen(name), error;
    if (!utf8_validate((const unsigned char *)name,length,&error)) { return relocation_fail(preview,RELOCATION_UNSUPPORTED_PATH,path,NULL); }
    bool plus = length != 0 && name[length-1] == '+';
    char *base = source_arena_copy(&preview->arena,name);
    if (base == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    if (plus) { base[length-1] = 0; }
    if (!unicode_name((const unsigned char *)base,strlen(base),&error)) { return true; }
    const char *physical = NULL;
    struct stat metadata;
    PathProbe probe = relocation_probe(preview,path,&physical,&metadata);
    if (probe != PATH_FOUND) { return relocation_fail(preview,probe == PATH_UNSUPPORTED
        ? RELOCATION_UNSUPPORTED_PATH : RELOCATION_IO_ERROR,path,NULL); }
    const char *logical = source_arena_join(&preview->arena,directory->logical,".",base);
    if (logical == NULL) { return relocation_fail(preview,RELOCATION_OUT_OF_MEMORY,path,NULL); }
    if (S_ISDIR(metadata.st_mode)) {
        return (!plus || anchor(preview,directory->path,base)) && relocation_queue(walk,path,relative,logical);
    }
    if (S_ISREG(metadata.st_mode)) {
        return plus || relocation_final_module(preview,physical,path,logical,&metadata);
    }
    return relocation_fail(preview,RELOCATION_UNSUPPORTED_PATH,path,NULL);
}
