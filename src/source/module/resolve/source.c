/**
 * @file
 * @brief Load unique module source bytes without selecting native companions.
 *
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) associates each ordinary
 * source with its selected version configuration.
 * Native companion selection follows parsed activation; all source/partial storage stays catalog-owned.
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
 * Component contract: [SRC-01](../../../../spec/language/source-and-names.md#src-01). */
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
    source->version = module_version_find(catalog, logical);
    source->device = metadata->st_dev;
    source->inode = metadata->st_ino;
    status = module_load_utf8(catalog, physical, &source->text);
    if (status == MODULE_OK) {
        source->next = catalog->first;
        catalog->first = source;
    }
    return status;
}
