/** @file
 * @brief Reuse production physical discovery and preserve its retained diagnostic context.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) rejects aliases
 * and duplicate identities; [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires unreadable sources to fail preparation without filesystem mutation.
 */
#include "../internal.h"
#include "diagnostics/adapters/adapters.h"

/** Catalog issue paths/source spans already have preview lifetime; retain one embedded related record without allocating during failure reporting. Component contract: [SRC-02](../../../../spec/language/source-and-names.md#src-02), [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01). */
bool relocation_catalog_failure(RelocationPreview *preview, const ModuleCatalog *catalog, ModuleStatus status) {
    if (status == MODULE_OK) { return true; }
    RelocationStatus failure = status == MODULE_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY
        : status == MODULE_AMBIGUOUS ? RELOCATION_COLLISION : status == MODULE_FILE_ALIAS ? RELOCATION_ALIAS
        : status == MODULE_ORPHAN_DIRECTORY ? RELOCATION_ORPHAN : status == MODULE_IO_ERROR ? RELOCATION_IO_ERROR
        : status == MODULE_UNSUPPORTED_PATH ? RELOCATION_UNSUPPORTED_PATH : RELOCATION_INVALID_SOURCE;
    (void)diagnostic_from_module(&catalog->issue,&preview->catalog_related,&preview->diagnostic);
    return relocation_fail(preview,failure,catalog->issue.path,catalog->issue.related_path);
}

/** The ordinary reader sorts exact names and closes streams on all allocation/read failures. Component contract: [SRC-02](../../../../spec/language/source-and-names.md#src-02), [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01). */
bool relocation_entries(RelocationPreview *preview, const char *path, const char ***names, size_t *count) {
    return relocation_catalog_failure(preview,&preview->catalog,module_catalog_entries(&preview->catalog,path,names,count));
}
