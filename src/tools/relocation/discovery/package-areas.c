/**
 * @file
 * @brief Index required areas of selected projected versions without external fallback.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) requires metadata/src/tests;
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) makes one complete copy
 * authoritative during [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * projected source discovery.
 */
#include "internal.h"

/** Require one exact projected entry's kind, retaining view-owned diagnostics.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) fixes required entries;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * never creates missing final paths while observing their original metadata.
 */
static bool required_entry(RelocationPreview *preview, const char *path,
    bool directory, struct stat *metadata) {
    const char *physical;
    PathProbe probe = relocation_probe(preview, path, &physical, metadata);
    if (probe != PATH_FOUND) {
        return relocation_fail(preview, probe == PATH_UNSUPPORTED ? RELOCATION_UNSUPPORTED_PATH
            : probe == PATH_IO_ERROR ? RELOCATION_IO_ERROR : RELOCATION_INVALID_SOURCE, path, NULL);
    }
    if (directory ? !S_ISDIR(metadata->st_mode) : !S_ISREG(metadata->st_mode)) {
        return relocation_fail(preview, RELOCATION_UNSUPPORTED_PATH, path, NULL);
    }
    return true;
}

/** Preserve complete area identity and apply exclusions after version selection.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) retains src/tests;
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) forbids filling excluded
 * or missing local modules from an ignored external version.
 */
static bool index_area(RelocationPreview *preview, const ModulePackage *package,
    const char *area) {
    const char *relative = source_arena_join(&preview->arena, package->relative, "/", area);
    const char *physical = source_arena_join(&preview->arena, package->physical, "/", area);
    const char *logical = source_arena_join(&preview->arena, package->logical, "/", area);
    if (relative == NULL || physical == NULL || logical == NULL) {
        return relocation_fail(preview, RELOCATION_OUT_OF_MEMORY, package->physical, NULL);
    }
    if (module_path_excluded(package->root, relative)) { return true; }
    struct stat metadata;
    return required_entry(preview, physical, true, &metadata)
        && relocation_walk_region(preview, package->root, physical, relative, logical);
}

/** Register selected version storage before its two required areas. Shared
 * metadata insertion detects physical aliases across standard roots, package
 * namespaces and areas under [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 * Configuration contents are not parsed by this discovery operation.
 */
bool relocation_walk_package(RelocationPreview *preview, const ModulePackage *package) {
    if (module_path_excluded(package->root, package->relative)) { return true; }
    struct stat metadata;
    if (!required_entry(preview, package->physical, true, &metadata)) { return false; }
    ModuleStatus status = module_remember_directory_metadata(&preview->after_catalog, package->physical, &metadata);
    if (!relocation_catalog_failure(preview, &preview->after_catalog, status)) { return false; }
    const char *configuration = source_arena_join(&preview->arena, package->physical, "/", "package.ini");
    if (configuration == NULL) {
        return relocation_fail(preview, RELOCATION_OUT_OF_MEMORY, package->physical, NULL);
    }
    return required_entry(preview, configuration, false, &metadata)
        && index_area(preview, package, "src") && index_area(preview, package, "tests");
}
