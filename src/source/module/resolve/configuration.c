/** @file
 * @brief Capture one required module's alias companion exactly once, including absence.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) fixes the physical .ini
 * file; [DEP-03](../../../../spec/implementation/compiler.md#dep-03) retains its
 * original bytes/presence for downstream configuration and invalidation consumers.
 */
#include "../internal.h"

/** Complete the observation before linking any cached configuration fields.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) keeps missing and empty
 * files distinct; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * reuses captured views instead of reopening mutable physical inputs.
 */
ModuleStatus module_catalog_load_configuration(ModuleCatalog *catalog, const ModuleSource *requested) {
    if (catalog == NULL || !catalog->built || requested == NULL) { return MODULE_INVALID_PATH; }
    ModuleSource *source = catalog->first;
    while (source != NULL && source != requested) { source = source->next; }
    if (source == NULL) { return MODULE_INVALID_PATH; }
    if (source->configuration_loaded) { return MODULE_OK; }
    ModuleCompanion companion;
    ModuleStatus status = module_read_companion(catalog, source_path(source->text), ".ini", true, &companion);
    if (status == MODULE_OK) {
        source->configuration_path = companion.path;
        source->configuration = companion.text;
        source->configuration_loaded = true;
    }
    return status;
}
