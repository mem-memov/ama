/**
 * @file
 * @brief Compose exact project-root validation with local-first version selection.
 *
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) applies precedence before
 * module indexing. Two complete passes make that choice independent of root
 * enumeration and prevent an ignored external copy from supplying any inputs.
 */
#include "internal.h"

/** Verify root kinds and ordinary physical identities before enumerating candidates.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) rejects repeated
 * physical roots even when they contain no source modules or package versions.
 */
static ModuleStatus verify_roots(ModuleCatalog *catalog, const ModuleRoot *roots,
    size_t count, const ModulePackageReader *reader) {
    for (size_t index = 0; index < count; index++) {
        if (roots[index].kind != MODULE_ROOT_PROJECT && roots[index].kind != MODULE_ROOT_STANDARD) {
            return module_fail(catalog, MODULE_INVALID_PATH, NULL);
        }
        if (roots[index].kind == MODULE_ROOT_STANDARD) {
            continue;
        }
        if (roots[index].path == NULL || roots[index].path[0] == 0) {
            return module_fail(catalog, MODULE_INVALID_PATH, NULL);
        }
        const char *path = source_arena_copy(&catalog->arena, roots[index].path);
        if (path == NULL) {
            return module_fail(catalog, MODULE_OUT_OF_MEMORY, NULL);
        }
        struct stat metadata;
        ModuleStatus status = reader->root(reader->context, catalog, path, &metadata);
        if (status == MODULE_OK) {
            status = module_remember_directory_metadata(catalog, path, &metadata);
        }
        if (status != MODULE_OK) {
            return status;
        }
    }
    return MODULE_OK;
}

/** Publish only a complete sorted selection; partial work stays catalog-owned.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) makes local selection a
 * version-level decision, so no source/configuration is loaded during either pass.
 */
ModuleStatus module_packages_select_view(ModuleCatalog *catalog, const ModuleRoot *roots,
    size_t count, const ModulePackageReader *reader, ModulePackage **output) {
    if (catalog == NULL || output == NULL || reader == NULL
        || reader->root == NULL || reader->child == NULL || reader->entries == NULL
        || (count != 0 && roots == NULL)
        || catalog->first != NULL || catalog->directories != NULL || catalog->built) {
        return MODULE_INVALID_PATH;
    }
    ModuleStatus status = verify_roots(catalog, roots, count, reader);
    ModulePackage *selected = NULL;
    for (size_t pass = 0; status == MODULE_OK && pass < 2; pass++) {
        for (size_t index = 0; status == MODULE_OK && index < count; index++) {
            if (roots[index].kind == MODULE_ROOT_PROJECT) {
                PackageScan scan = {.catalog = catalog, .reader = reader, .root = &roots[index],
                    .selected = &selected, .local = pass == 0};
                status = package_discover(&scan);
            }
        }
    }
    if (status == MODULE_OK) {
        *output = selected;
    }
    return status;
}
