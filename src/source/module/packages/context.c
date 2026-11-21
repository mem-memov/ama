/** @file
 * @brief Retain selected version configuration independently of build-borrowed roots.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) chooses one whole version;
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) makes its exact configuration
 * authoritative. [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * supplies canonical version boundaries, including distinct version variants.
 */
#include <string.h>
#include "../internal.h"
#include "source/path/logical.h"

/** Match only the complete version boundary, never a prefix of a different version.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) determines the
 * boundary; [PKG-02](../../../../spec/project/packages.md#pkg-02) fixes its owner.
 */
const ModuleVersion *module_version_find(const ModuleCatalog *catalog, const char *logical) {
    ModulePathShape shape;
    size_t length = strlen(logical);
    if (!module_path_describe((const unsigned char *)logical, length, &shape)
        || shape.root != MODULE_ROOT_PROJECT || shape.version_length == 0) {
        return NULL;
    }
    for (const ModuleVersion *version = catalog->versions; version != NULL; version = version->next) {
        if (strlen(version->logical_path) == shape.version_length
            && memcmp(version->logical_path, logical, shape.version_length) == 0) {
            return version;
        }
    }
    return NULL;
}

/** Prepare the entire owner before publishing any context/module link.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) keeps the original source
 * location and bytes; [PKG-02](../../../../spec/project/packages.md#pkg-02) refuses
 * a second owner rather than replacing selected configuration. All scratch is
 * catalog-owned on failure. Runtime is O(V + M) identity checks plus their bytes;
 * successful storage is one record and two copied names, without duplicating text.
 */
ModuleStatus module_version_publish(ModuleCatalog *catalog, const char *logical,
    const char *physical, const SourceText *configuration) {
    ModulePathShape shape;
    if (logical == NULL || physical == NULL || physical[0] == 0 || configuration == NULL
        || !module_path_describe((const unsigned char *)logical, strlen(logical), &shape)
        || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) {
        return module_fail(catalog, MODULE_INVALID_PATH, NULL);
    }
    const ModuleVersion *previous = module_version_find(catalog, logical);
    if (previous != NULL) {
        catalog->issue.related_path = source_path(previous->configuration);
        return module_fail(catalog, MODULE_AMBIGUOUS, source_path(configuration));
    }
    ModuleVersion *version = source_arena_allocate(&catalog->arena, sizeof(*version));
    const char *identity = source_arena_copy(&catalog->arena, logical);
    const char *directory = source_arena_copy(&catalog->arena, physical);
    if (version == NULL || identity == NULL || directory == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, source_path(configuration));
    }
    *version = (ModuleVersion){.next = catalog->versions, .logical_path = identity,
        .physical_path = directory, .configuration = configuration};
    catalog->versions = version;
    for (ModuleSource *source = catalog->first; source != NULL; source = source->next) {
        ModulePathShape source_shape;
        size_t source_length = strlen(source->logical_path);
        if (module_path_describe((const unsigned char *)source->logical_path,
                source_length, &source_shape)
            && source_shape.root == MODULE_ROOT_PROJECT
            && source_shape.version_length == shape.version_length
            && memcmp(source->logical_path, identity, shape.version_length) == 0) {
            source->version = version;
        }
    }
    return MODULE_OK;
}
