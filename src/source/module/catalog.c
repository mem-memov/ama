/**
 * @file
 * @brief Guard one catalog build and expose lookup only after complete success.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Duplicate logical sources fail instead of selecting whichever root was visited first.
 */
#include <string.h>
#include "internal.h"
#include "packages/packages.h"

void module_catalog_init(ModuleCatalog *catalog, SourceAllocator allocator) {
    *catalog = (ModuleCatalog){0};
    source_arena_init(&catalog->arena, allocator);
    source_manager_init(&catalog->sources, allocator);
}

void module_catalog_destroy(ModuleCatalog *catalog) {
    source_manager_destroy(&catalog->sources);
    source_arena_destroy(&catalog->arena);
    catalog->first = NULL;
    catalog->directories = NULL;
    catalog->built = false;
    catalog->issue = (ModuleIssue){0};
}

ModuleStatus module_fail(ModuleCatalog *catalog, ModuleStatus status, const char *path) {
    catalog->issue.status = status;
    catalog->issue.path = path;
    return status;
}


ModuleStatus module_catalog_build(ModuleCatalog *catalog,
    const ModuleRoot *roots, size_t root_count) {
    if (catalog->attempted || (root_count != 0 && roots == NULL)) {
        return MODULE_INVALID_PATH;
    }
    catalog->attempted = true;
    ModulePackage *packages = NULL;
    ModuleStatus status = module_packages_select(catalog, roots, root_count, &packages);
    if (status != MODULE_OK) {
        return status;
    }
    for (size_t index = 0; index < root_count; index++) {
        if (roots[index].kind == MODULE_ROOT_STANDARD) {
            status = module_walk_root(catalog, &roots[index]);
            if (status != MODULE_OK) {
                return status;
            }
        }
    }
    for (const ModulePackage *package = packages; package != NULL; package = package->next) {
        status = module_package_index(catalog, package);
        if (status != MODULE_OK) {
            return status;
        }
    }
    catalog->built = true;
    return MODULE_OK;
}

ModuleStatus module_catalog_find(const ModuleCatalog *catalog,
    const char *logical_path, const ModuleSource **output) {
    if (!catalog->built || !module_path_valid(logical_path)) {
        return MODULE_INVALID_PATH;
    }
    for (const ModuleSource *source = catalog->first; source != NULL; source = source->next) {
        if (strcmp(source->logical_path, logical_path) == 0) {
            *output = source;
            return MODULE_OK;
        }
    }
    return MODULE_MISSING;
}
