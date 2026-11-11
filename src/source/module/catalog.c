#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

void module_catalog_init(ModuleCatalog *catalog, SourceAllocator allocator) {
    *catalog = (ModuleCatalog){0};
    source_arena_init(&catalog->arena, allocator);
    source_manager_init(&catalog->sources, allocator);
}

void module_catalog_destroy(ModuleCatalog *catalog) {
    source_manager_destroy(&catalog->sources);
    source_arena_destroy(&catalog->arena);
    catalog->first = NULL;
    catalog->directories = NULL;
    catalog->built = false;
    catalog->issue = (ModuleIssue){0};
}

ModuleStatus module_fail(ModuleCatalog *catalog, ModuleStatus status, const char *path) {
    catalog->issue.status = status;
    catalog->issue.path = path;
    return status;
}

bool module_path_valid(const char *path) {
    if (path == NULL || path[0] != '.' || path[1] == 0) {
        return false;
    }
    const char *start = path + 1;
    do {
        const char *end = strchr(start, '.');
        size_t length = end == NULL ? strlen(start) : (size_t)(end - start);
        size_t error;
        if (!unicode_identifier((const unsigned char *)start, length, &error)) {
            return false;
        }
        if (end == NULL) {
            return true;
        }
        start = end + 1;
    } while (true);
}

ModuleStatus module_catalog_build(ModuleCatalog *catalog,
    const ModuleRoot *roots, size_t root_count) {
    if (catalog->attempted) {
        return MODULE_INVALID_PATH;
    }
    catalog->attempted = true;
    for (size_t index = 0; index < root_count; index++) {
        ModuleStatus status = module_walk_root(catalog, &roots[index]);
        if (status != MODULE_OK) {
            return status;
        }
    }
    catalog->built = true;
    return MODULE_OK;
}

ModuleStatus module_catalog_find(const ModuleCatalog *catalog,
    const char *logical_path, const ModuleSource **output) {
    if (!catalog->built || !module_path_valid(logical_path)) {
        return MODULE_INVALID_PATH;
    }
    for (const ModuleSource *source = catalog->first; source != NULL; source = source->next) {
        if (strcmp(source->logical_path, logical_path) == 0) {
            *output = source;
            return MODULE_OK;
        }
    }
    return MODULE_MISSING;
}
