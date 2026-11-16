/**
 * @file
 * @brief Load physical native companions only after the caller establishes native activation.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) selects
 * companions for active native modules; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * requires each contributing input once. Complete companion views publish atomically.
 */
#include <string.h>
#include "../internal.h"

/** Probe a named optional C/header/ABI companion with exact bytes; missing is valid, unsupported entries fail, ABI text is loaded/UTF-8 checked.
 * Component contract: [SRC-01](../../../../spec/language/source-and-names.md#src-01), [DEP-03](../../../../spec/implementation/compiler.md#dep-03). */
static ModuleStatus load_companion(ModuleCatalog *catalog, const char *physical,
    const char *suffix, const char **output, const SourceText **text) {
    const char *path = source_arena_join(&catalog->arena, physical, "", suffix);
    char *parent = source_arena_copy(&catalog->arena, physical);
    if (path == NULL || parent == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    char *slash = strrchr(parent, '/');
    const char *name = strrchr(path, '/');
    if (slash != NULL) {
        *slash = 0;
    }
    struct stat metadata;
    PathProbe probe = path_exact_child(slash == NULL ? "." : (*parent == 0 ? "/" : parent),
        name == NULL ? path : name + 1, path, &metadata);
    if (probe == PATH_MISSING) {
        return MODULE_OK;
    }
    if (probe != PATH_FOUND || !S_ISREG(metadata.st_mode)) {
        return module_fail(catalog, probe == PATH_IO_ERROR ? MODULE_IO_ERROR
            : MODULE_UNSUPPORTED_PATH, path);
    }
    *output = path;
    return text == NULL ? MODULE_OK : module_load_utf8(catalog, path, text);
}

ModuleStatus module_catalog_load_native(ModuleCatalog *catalog, const ModuleSource *requested) {
    if (!catalog->built || requested == NULL) { return MODULE_INVALID_PATH; }
    ModuleSource *source = catalog->first;
    while (source != NULL && source != requested) { source = source->next; }
    if (source == NULL) { return MODULE_INVALID_PATH; }
    if (source->native_loaded) { return MODULE_OK; }
    const char *physical = source_path(source->text);
    const char *body = NULL, *header = NULL, *abi_path = NULL;
    const SourceText *abi = NULL;
    ModuleStatus status = load_companion(catalog, physical, ".c", &body, NULL);
    if (status == MODULE_OK) { status = load_companion(catalog, physical, ".h", &header, NULL); }
    if (status == MODULE_OK) { status = load_companion(catalog, physical, ".abi", &abi_path, &abi); }
    if (status == MODULE_OK) {
        source->native_c = body;
        source->native_header = header;
        source->native_abi = abi;
        source->native_loaded = true;
    }
    return status;
}
