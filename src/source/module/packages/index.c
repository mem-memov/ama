/**
 * @file
 * @brief Index one selected version's required source/test areas without fallback.
 *
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) requires the version's
 * metadata and source/test areas. [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * makes missing local inputs errors instead of requests to inspect another copy.
 */
#include "internal.h"

/** Require an exact regular package.ini without parsing its configuration here.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) determines its presence;
 * configuration semantics belong to the separate PKG-03 consumer.
 */
static ModuleStatus require_configuration(ModuleCatalog *catalog, const ModulePackage *package) {
    const char *path = source_arena_join(&catalog->arena, package->physical, "/", "package.ini");
    if (path == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, package->physical);
    }
    struct stat metadata;
    ModuleStatus status = module_probe(catalog, package->physical, "package.ini", path, &metadata);
    return status != MODULE_OK || S_ISREG(metadata.st_mode) ? status
        : module_fail(catalog, MODULE_UNSUPPORTED_PATH, path);
}

/** Prepare exact area prefixes and delegate only this selected copy's module scan.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) retains src/tests
 * in logical identity; [SRC-05](../../../../spec/language/source-and-names.md#src-05)
 * leaves structural plus directories to the common module scanner.
 */
static ModuleStatus index_area(ModuleCatalog *catalog, const ModulePackage *package,
    const char *area) {
    const char *relative = source_arena_join(&catalog->arena, package->relative, "/", area);
    const char *physical = source_arena_join(&catalog->arena, package->physical, "/", area);
    const char *logical = source_arena_join(&catalog->arena, package->logical, "/", area);
    if (relative == NULL || physical == NULL || logical == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, package->physical);
    }
    if (module_path_excluded(package->root, relative)) {
        return MODULE_OK;
    }
    struct stat metadata;
    ModuleStatus status = module_probe(catalog, package->physical, area, physical, &metadata);
    if (status != MODULE_OK || !S_ISDIR(metadata.st_mode)) {
        return status != MODULE_OK ? status
            : module_fail(catalog, MODULE_UNSUPPORTED_PATH, physical);
    }
    return module_walk_region(catalog, package->root, physical, relative, logical);
}

/** Validate/index only after selection; no caller-owned path enters retained issues.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) prevents exclusions and
 * missing module/configuration files from filling holes from ignored copies.
 */
ModuleStatus module_package_index(ModuleCatalog *catalog, const ModulePackage *package) {
    if (module_path_excluded(package->root, package->relative)) {
        return MODULE_OK;
    }
    ModuleStatus status = module_remember_directory(catalog, package->physical);
    if (status == MODULE_OK) {
        status = require_configuration(catalog, package);
    }
    if (status == MODULE_OK) {
        status = index_area(catalog, package, "src");
    }
    return status == MODULE_OK ? index_area(catalog, package, "tests") : status;
}
