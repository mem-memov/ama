/** @file
 * @brief Copy the authoritative optional module configuration in a captured source entry.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) fixes immutable UTF-8
 * companion bytes. [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * distinguishes absent and empty configuration. No projected physical input is read.
 */
#include "../internal.h"
#include "source/unicode/unicode.h"

/** Prepare the complete optional observation before changing source fields.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) preserves original paths
 * and bytes; [DEP-03](../../../../spec/implementation/compiler.md#dep-03) seals
 * successful absence so a later activation cannot reread mutable disk state.
 * Failed owned scratch/encoding diagnostics survive until catalog destruction.
 */
ModuleStatus module_snapshot_aliases(ModuleCatalog *catalog, ModuleSource *source,
    const SourceText *configuration) {
    const SourceText *copied = NULL;
    const char *path;
    if (configuration != NULL) {
        if (source_manager_add(&catalog->sources, source_path(configuration),
                source_bytes(configuration), source_length(configuration), &copied) != SOURCE_OK) {
            return module_fail(catalog, MODULE_OUT_OF_MEMORY, source_path(source->text));
        }
        size_t offset;
        if (!utf8_validate(source_bytes(copied), source_length(copied), &offset)) {
            catalog->issue.source = copied;
            catalog->issue.offset = offset;
            return module_fail(catalog, MODULE_INVALID_UTF8, source_path(copied));
        }
        path = source_path(copied);
    } else {
        path = source_arena_join(&catalog->arena, source_path(source->text), "", ".ini");
        if (path == NULL) { return module_fail(catalog, MODULE_OUT_OF_MEMORY, source_path(source->text)); }
    }
    source->configuration_path = path;
    source->configuration = copied;
    source->configuration_loaded = true;
    return MODULE_OK;
}
