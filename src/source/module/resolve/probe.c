#include <string.h>
#include "../internal.h"
#include "source/unicode/unicode.h"

ModuleStatus module_probe(ModuleCatalog *catalog, const char *parent,
    const char *name, const char *full, struct stat *metadata) {
    PathProbe probe = path_exact_child(parent, name, full, metadata);
    if (probe == PATH_FOUND) {
        return MODULE_OK;
    }
    ModuleStatus status = probe == PATH_MISSING ? MODULE_MISSING
        : probe == PATH_UNSUPPORTED ? MODULE_UNSUPPORTED_PATH : MODULE_IO_ERROR;
    return module_fail(catalog, status, full);
}

ModuleStatus module_verify_root(ModuleCatalog *catalog, const char *path) {
    size_t error;
    if (path == NULL || *path == 0
        || !utf8_validate((const unsigned char *)path, strlen(path), &error)) {
        return module_fail(catalog, MODULE_INVALID_PATH, NULL);
    }
    char *copy = source_arena_copy(&catalog->arena, path);
    const char *parent = path[0] == '/' ? "/" : ".";
    if (copy == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, NULL);
    }
    char *cursor = copy;
    while (*cursor != 0) {
        if (*cursor == '/') {
            cursor++;
            continue;
        }
        char *name = cursor;
        while (*cursor != 0 && *cursor != '/') {
            cursor++;
        }
        char separator = *cursor;
        *cursor = 0;
        const char *full = source_arena_join(&catalog->arena, parent, "/", name);
        if (full == NULL) {
            return module_fail(catalog, MODULE_OUT_OF_MEMORY, parent);
        }
        struct stat metadata;
        ModuleStatus status = module_probe(catalog, parent, name, full, &metadata);
        if (status != MODULE_OK || !S_ISDIR(metadata.st_mode)) {
            return status != MODULE_OK ? status
                : module_fail(catalog, MODULE_UNSUPPORTED_PATH, full);
        }
        parent = full;
        if (separator != 0) {
            cursor++;
        }
    }
    return MODULE_OK;
}
