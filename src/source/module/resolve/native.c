/**
 * @file
 * @brief Load physical native companions only after the caller establishes native activation.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) selects
 * companions for active native modules; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * requires each contributing input once. Complete companion views publish atomically.
 */
#include "../internal.h"

ModuleStatus module_catalog_load_native(ModuleCatalog *catalog, const ModuleSource *requested) {
    if (!catalog->built || requested == NULL) { return MODULE_INVALID_PATH; }
    ModuleSource *source = catalog->first;
    while (source != NULL && source != requested) { source = source->next; }
    if (source == NULL) { return MODULE_INVALID_PATH; }
    if (source->native_loaded) { return MODULE_OK; }
    const char *physical = source_path(source->text);
    ModuleCompanion body, header, abi;
    ModuleStatus status = module_read_companion(catalog, physical, ".c", false, &body);
    if (status == MODULE_OK) { status = module_read_companion(catalog, physical, ".h", false, &header); }
    if (status == MODULE_OK) { status = module_read_companion(catalog, physical, ".abi", true, &abi); }
    if (status == MODULE_OK) {
        source->native_c = body.present ? body.path : NULL;
        source->native_header = header.present ? header.path : NULL;
        source->native_abi = abi.text;
        source->native_loaded = true;
    }
    return status;
}
