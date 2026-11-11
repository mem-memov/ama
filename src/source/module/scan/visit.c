#include <string.h>
#include "../internal.h"
#include "source/unicode/unicode.h"

static bool excluded(const ModuleRoot *root, const char *relative) {
    for (size_t index = 0; index < root->excluded_count; index++) {
        size_t length = strlen(root->excluded[index]);
        if (strncmp(relative, root->excluded[index], length) == 0
            && (relative[length] == 0 || relative[length] == '/')) {
            return true;
        }
    }
    return false;
}

static ModuleStatus verify_anchor(ModuleWalk *walk, const ModuleDirectory *directory,
    const char *base) {
    const char *anchor = source_arena_join(&walk->catalog->arena, directory->physical, "/", base);
    if (anchor == NULL) {
        return module_fail(walk->catalog, MODULE_OUT_OF_MEMORY, directory->physical);
    }
    struct stat metadata;
    ModuleStatus status = module_probe(walk->catalog, directory->physical, base, anchor, &metadata);
    if (status == MODULE_MISSING || (status == MODULE_OK && !S_ISREG(metadata.st_mode))) {
        return module_fail(walk->catalog, MODULE_ORPHAN_DIRECTORY, anchor);
    }
    return status;
}

static ModuleStatus visit_candidate(ModuleWalk *walk, const ModuleDirectory *directory,
    const char *name, const char *base, bool submodule,
    const char *physical, const char *relative) {
    struct stat metadata;
    ModuleStatus status = module_probe(walk->catalog, directory->physical, name, physical, &metadata);
    if (status != MODULE_OK) {
        return status;
    }
    const char *logical = source_arena_join(&walk->catalog->arena, directory->logical, ".", base);
    if (logical == NULL) {
        return module_fail(walk->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    if (S_ISDIR(metadata.st_mode)) {
        status = submodule ? verify_anchor(walk, directory, base) : MODULE_OK;
        return status == MODULE_OK ? module_queue(walk, physical, relative, logical) : status;
    }
    if (S_ISREG(metadata.st_mode) && !submodule) {
        return module_add_source(walk->catalog, physical, logical, &metadata);
    }
    return submodule && S_ISREG(metadata.st_mode) ? MODULE_OK
        : module_fail(walk->catalog, MODULE_UNSUPPORTED_PATH, physical);
}

ModuleStatus module_visit_entry(ModuleWalk *walk, const ModuleDirectory *directory,
    const char *name) {
    const char *relative = source_arena_join(&walk->catalog->arena,
        directory->relative, *directory->relative == 0 ? "" : "/", name);
    const char *physical = source_arena_join(&walk->catalog->arena, directory->physical, "/", name);
    if (relative == NULL || physical == NULL) {
        return module_fail(walk->catalog, MODULE_OUT_OF_MEMORY, directory->physical);
    }
    if (excluded(walk->root, relative)) {
        return MODULE_OK;
    }
    size_t length = strlen(name);
    bool submodule = length != 0 && name[length - 1] == '+';
    char *base = source_arena_copy(&walk->catalog->arena, name);
    if (base == NULL) {
        return module_fail(walk->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    if (submodule) {
        base[length - 1] = 0;
    }
    size_t error;
    if (!utf8_validate((const unsigned char *)name, length, &error)) {
        return module_fail(walk->catalog, MODULE_UNSUPPORTED_PATH, physical);
    }
    if (!unicode_identifier((const unsigned char *)base, strlen(base), &error)) {
        return MODULE_OK;
    }
    return visit_candidate(walk, directory, name, base, submodule, physical, relative);
}
