/**
 * @file
 * @brief Load unique source/ABI bytes and associate checked native companions.
 *
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * Publish a source record only after its checked companions load; all partial storage stays catalog-owned.
 */
#include <string.h>
#include "../internal.h"
#include "source/unicode/unicode.h"

ModuleStatus module_load_utf8(ModuleCatalog *catalog, const char *physical,
    const SourceText **output) {
    SourceStatus loaded = source_manager_load(&catalog->sources, physical, output);
    if (loaded != SOURCE_OK) {
        return module_fail(catalog, loaded == SOURCE_IO_ERROR ? MODULE_IO_ERROR
            : MODULE_OUT_OF_MEMORY, physical);
    }
    size_t error;
    if (!utf8_validate(source_bytes(*output), source_length(*output), &error)) {
        catalog->issue.source = *output;
        catalog->issue.offset = error;
        return module_fail(catalog, MODULE_INVALID_UTF8, source_path(*output));
    }
    return MODULE_OK;
}

/** Compare complete logical names and exact device/inode pairs; retain the prior physical path for either ambiguity or alias.
 */
static ModuleStatus check_unique(ModuleCatalog *catalog, const char *physical,
    const char *logical, const struct stat *metadata) {
    for (ModuleSource *other = catalog->first; other != NULL; other = other->next) {
        bool same_name = strcmp(other->logical_path, logical) == 0;
        bool same_file = other->device == metadata->st_dev && other->inode == metadata->st_ino;
        if (same_name || same_file) {
            catalog->issue.related_path = source_path(other->text);
            return module_fail(catalog, same_name ? MODULE_AMBIGUOUS : MODULE_FILE_ALIAS, physical);
        }
    }
    return MODULE_OK;
}

/** Probe a named optional C/header/ABI companion with exact bytes; missing is valid, unsupported entries fail, ABI text is loaded/UTF-8 checked.
 */
static ModuleStatus load_companion(ModuleCatalog *catalog, const char *physical,
    const char *suffix, const char **output, const SourceText **text) {
    const char *path = source_arena_join(&catalog->arena, physical, "", suffix);
    char *parent = source_arena_copy(&catalog->arena, physical);
    if (path == NULL || parent == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    char *slash = strrchr(parent, '/');
    const char *name = strrchr(path, '/');
    if (slash != NULL) {
        *slash = 0;
    }
    struct stat metadata;
    PathProbe probe = path_exact_child(slash == NULL ? "." : (*parent == 0 ? "/" : parent),
        name == NULL ? path : name + 1, path, &metadata);
    if (probe == PATH_MISSING) {
        return MODULE_OK;
    }
    if (probe != PATH_FOUND || !S_ISREG(metadata.st_mode)) {
        return module_fail(catalog, probe == PATH_IO_ERROR ? MODULE_IO_ERROR
            : MODULE_UNSUPPORTED_PATH, path);
    }
    *output = path;
    return text == NULL ? MODULE_OK : module_load_utf8(catalog, path, text);
}

ModuleStatus module_add_source(ModuleCatalog *catalog, const char *physical,
    const char *logical, const struct stat *metadata) {
    ModuleStatus status = check_unique(catalog, physical, logical, metadata);
    if (status != MODULE_OK) {
        return status;
    }
    ModuleSource *source = source_arena_allocate(&catalog->arena, sizeof(*source));
    if (source == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    *source = (ModuleSource){0};
    source->logical_path = logical;
    source->device = metadata->st_dev;
    source->inode = metadata->st_ino;
    status = module_load_utf8(catalog, physical, &source->text);
    if (status == MODULE_OK) {
        status = load_companion(catalog, physical, ".c", &source->native_c, NULL);
    }
    if (status == MODULE_OK) {
        status = load_companion(catalog, physical, ".h", &source->native_header, NULL);
    }
    if (status == MODULE_OK) {
        const char *path = NULL;
        status = load_companion(catalog, physical, ".abi", &path, &source->native_abi);
    }
    if (status == MODULE_OK) {
        source->next = catalog->first;
        catalog->first = source;
    }
    return status;
}
