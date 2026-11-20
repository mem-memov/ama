/**
 * @file
 * @brief Bind physical exact-path operations to the shared package selection algorithm.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) verifies root
 * components and child spellings before selection; projected tooling provides
 * its own reader under [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
#include "internal.h"

/** Root validation precedes metadata publication, preserving exact spelling and
 * symlink refusal under [SRC-02](../../../../spec/language/source-and-names.md#src-02).
 * The reader receives a catalog-owned path, so any retained failure remains valid.
 */
static ModuleStatus physical_root(void *context, ModuleCatalog *catalog,
    const char *path, struct stat *metadata) {
    (void)context;
    ModuleStatus status = module_verify_root(catalog, path);
    if (status != MODULE_OK) { return status; }
    return lstat(path, metadata) == 0 ? MODULE_OK : module_fail(catalog, MODULE_IO_ERROR, path);
}

/** Adapt the existing exact child probe without weakening its error provenance.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) excludes aliases
 * and missing entries; the finite selection driver owns all supplied strings.
 */
static ModuleStatus physical_child(void *context, ModuleCatalog *catalog,
    const char *parent, const char *name, const char *full, struct stat *metadata) {
    (void)context;
    return module_probe(catalog, parent, name, full, metadata);
}

/** Retain bytewise sorted entries and stream cleanup in their existing owner.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) requires the
 * selector to see a complete checked directory, not discovery-order guesses.
 */
static ModuleStatus physical_entries(void *context, ModuleCatalog *catalog,
    const char *path, const char ***names, size_t *count) {
    (void)context;
    return module_catalog_entries(catalog, path, names, count);
}

/** Install only physical reads for compiler selection; no reader state escapes.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) is implemented by the
 * shared selector, so native and projected callers cannot choose different copies.
 */
ModuleStatus module_packages_select(ModuleCatalog *catalog, const ModuleRoot *roots,
    size_t count, ModulePackage **output) {
    const ModulePackageReader reader = {.root = physical_root,
        .child = physical_child, .entries = physical_entries};
    return module_packages_select_view(catalog, roots, count, &reader, output);
}
