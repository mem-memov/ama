/** @file
 * @brief Attach captured package configuration without rereading projected filesystem paths.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) retains a single selected
 * version owner; [PKG-03](../../../../spec/project/packages.md#pkg-03) requires
 * well-formed UTF-8 and preserves configuration source locations.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires validation from prepared captured inputs, not stale physical files.
 */
#include <string.h>
#include "../internal.h"
#include "source/path/logical.h"
#include "source/unicode/unicode.h"

/** Copy source before publication so failures retain owned diagnostics and no live links change.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) defers INI interpretation
 * to the document parser; byte storage validates only UTF-8. [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * refuses duplicate owners. [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * prevents filesystem fallback. Existing module text/native views remain unchanged.
 */
ModuleStatus module_catalog_snapshot_configuration(ModuleCatalog *catalog, const char *logical,
    const char *physical, const SourceText *configuration) {
    ModulePathShape shape;
    if (catalog == NULL || !catalog->built || logical == NULL || physical == NULL
        || physical[0] == 0 || configuration == NULL
        || !module_path_describe((const unsigned char *)logical, strlen(logical), &shape)
        || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) {
        return MODULE_INVALID_PATH;
    }
    const SourceText *copied = NULL;
    SourceStatus status = source_manager_add(&catalog->sources, source_path(configuration),
        source_bytes(configuration), source_length(configuration), &copied);
    if (status != SOURCE_OK) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, NULL);
    }
    size_t offset;
    if (!utf8_validate(source_bytes(copied), source_length(copied), &offset)) {
        catalog->issue.source = copied;
        catalog->issue.offset = offset;
        return module_fail(catalog, MODULE_INVALID_UTF8, source_path(copied));
    }
    return module_version_publish(catalog, logical, physical, copied);
}
