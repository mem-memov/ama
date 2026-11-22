/**
 * @file
 * @brief Guard one catalog build and expose lookup only after complete success.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Duplicate logical sources fail instead of selecting whichever root was visited first.
 */
#include <string.h>
#include "internal.h"
#include "view.h"
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
    catalog->versions = NULL;
    catalog->directories = NULL;
    catalog->built = false;
    catalog->reader = NULL;
    catalog->issue = (ModuleIssue){0};
}

ModuleStatus module_fail(ModuleCatalog *catalog, ModuleStatus status, const char *path) {
    catalog->issue.status = status;
    catalog->issue.path = path;
    return status;
}


/** Keep one selector/scanner for physical and explicit prepared views.
 * [PKG-02](../../../spec/project/packages.md#pkg-02) applies before module
 * indexing in either case; [SRC-05](../../../spec/language/source-and-names.md#src-05)
 * keeps plus-anchor validation in the common traversal. No failed view falls back.
 */
static ModuleStatus build(ModuleCatalog *catalog, const ModuleRoot *roots,
    size_t root_count, const ModuleCatalogReader *reader) {
    if (!catalog || catalog->attempted || (root_count != 0 && roots == NULL)) {
        return MODULE_INVALID_PATH;
    }
    catalog->attempted = true;
    catalog->reader = reader;
    ModulePackage *packages = NULL;
    ModuleStatus status = reader
        ? module_packages_select_view(catalog, roots, root_count, &reader->tree, &packages)
        : module_packages_select(catalog, roots, root_count, &packages);
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

ModuleStatus module_catalog_build(ModuleCatalog *catalog, const ModuleRoot *roots, size_t count) {
    return build(catalog, roots, count, NULL);
}
ModuleStatus module_catalog_build_view(ModuleCatalog *catalog, const ModuleRoot *roots, size_t count,
    const ModuleCatalogReader *reader) {
    if (!reader || !reader->tree.root || !reader->tree.child || !reader->tree.entries || !reader->read) {
        return MODULE_INVALID_PATH;
    }
    return build(catalog, roots, count, reader);
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
