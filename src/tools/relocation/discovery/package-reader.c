/**
 * @file
 * @brief Adapt the projected filesystem to the common whole-version selector.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) fixes precedence in both
 * views; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * reads final locations through their original physical storage without mutation.
 */
#include "internal.h"

/** Retain the reader's owned diagnostic boundary in its separate selection
 * catalog. [SRC-02](../../../../spec/language/source-and-names.md#src-02) keeps
 * failure paths valid until their catalog/preview owner is destroyed.
 */
static ModuleStatus projected_failure(ModuleCatalog *catalog, ModuleStatus status,
    const char *path) {
    catalog->issue = (ModuleIssue){.status = status, .path = path};
    return status;
}

/** Translate an exact projected probe without losing a preview-owned first cause.
 * Roots have already passed complete path validation in relocation_roots. Child
 * probes still verify exact original entry spellings through relocation_probe.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) forbids alias or
 * missing-entry repair; successful metadata belongs to the selected view.
 */
static ModuleStatus projected_metadata(void *context, ModuleCatalog *catalog,
    const char *path, struct stat *metadata) {
    RelocationPreview *preview = context;
    const char *physical;
    PathProbe probe = relocation_probe(preview, path, &physical, metadata);
    if (probe == PATH_FOUND) { return MODULE_OK; }
    ModuleStatus status = preview->status == RELOCATION_OUT_OF_MEMORY ? MODULE_OUT_OF_MEMORY
        : probe == PATH_MISSING ? MODULE_MISSING
        : probe == PATH_UNSUPPORTED ? MODULE_UNSUPPORTED_PATH : MODULE_IO_ERROR;
    return projected_failure(catalog, status, path);
}

/** The selector constructs each child from an exact parent and entry name; the
 * projected probe validates its mapped original full path under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static ModuleStatus projected_child(void *context, ModuleCatalog *catalog,
    const char *parent, const char *name, const char *full, struct stat *metadata) {
    (void)parent;
    (void)name;
    return projected_metadata(context, catalog, full, metadata);
}

/** Sorted projected names retain the preview lifetime, longer than selection.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * inserts/removes only exact moved endpoints, without a staged physical tree.
 */
static ModuleStatus projected_entries(void *context, ModuleCatalog *catalog,
    const char *path, const char ***names, size_t *count) {
    RelocationPreview *preview = context;
    if (relocation_view_entries(preview, path, names, count)) { return MODULE_OK; }
    return projected_failure(catalog, preview->status == RELOCATION_OUT_OF_MEMORY
        ? MODULE_OUT_OF_MEMORY : MODULE_IO_ERROR, path);
}

/** Retain selection scratch/errors until preview destruction and publish only a
 * complete version list. The shared driver performs local-before-external passes
 * under [PKG-02](../../../../spec/project/packages.md#pkg-02); no tool-specific
 * copy choice or fallback is added. Earlier preview failures remain authoritative.
 */
bool relocation_select_packages(RelocationPreview *preview, ModulePackage **output) {
    const ModulePackageReader reader = {.context = preview, .root = projected_metadata,
        .child = projected_child, .entries = projected_entries};
    ModuleStatus status = module_packages_select_view(&preview->after_catalog,
        preview->after_roots, preview->after_root_count, &reader, output);
    return preview->status == RELOCATION_OK
        && relocation_catalog_failure(preview, &preview->after_catalog, status);
}
