/**
 * @file
 * @brief Walk verified directory work records iteratively with explicit exclusions.
 *
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * Every physical directory is registered once; duplicate identities fail rather than cycling.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) keeps the standard
 * namespace separate. Ordinary roots reach this traversal only through selected
 * version areas under [PKG-02](../../../../spec/project/packages.md#pkg-02).
 */
#include "../internal.h"

ModuleStatus module_queue(ModuleWalk *walk, const char *physical,
    const char *relative, const char *logical) {
    ModuleStatus status = module_remember_directory(walk->catalog, physical);
    if (status != MODULE_OK) {
        return status;
    }
    ModuleDirectory *directory = source_arena_allocate(&walk->catalog->arena, sizeof(*directory));
    if (directory == NULL) {
        return module_fail(walk->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    *directory = (ModuleDirectory){NULL, physical, relative, logical};
    if (walk->last == NULL) {
        walk->pending = directory;
    } else {
        walk->last->next = directory;
    }
    walk->last = directory;
    return MODULE_OK;
}

ModuleStatus module_walk_root(ModuleCatalog *catalog, const ModuleRoot *root) {
    if (root->kind != MODULE_ROOT_STANDARD) {
        return module_fail(catalog, MODULE_INVALID_PATH, NULL);
    }
    ModuleStatus status = module_verify_root(catalog, root->path);
    if (status != MODULE_OK) {
        return status;
    }
    const char *physical = source_arena_copy(&catalog->arena, root->path);
    if (physical == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, NULL);
    }
    return module_walk_region(catalog, root, physical, "", "/");
}

/** Drain one selected area without reparsing its prefix or choosing other roots.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) gives each
 * component exactly one namespace; [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * guarantees that ordinary prefixes already refer to the sole selected copy.
 */
ModuleStatus module_walk_region(ModuleCatalog *catalog, const ModuleRoot *root,
    const char *physical, const char *relative, const char *logical) {
    ModuleWalk walk = {catalog, root, NULL, NULL};
    ModuleStatus status = module_queue(&walk, physical, relative, logical);
    while (status == MODULE_OK && walk.pending != NULL) {
        ModuleDirectory *directory = walk.pending;
        walk.pending = directory->next;
        if (walk.pending == NULL) {
            walk.last = NULL;
        }
        const char **names;
        size_t count;
        status = module_catalog_entries(catalog, directory->physical, &names, &count);
        for (size_t index = 0; status == MODULE_OK && index < count; index++) {
            status = module_visit_entry(&walk, directory, names[index]);
        }
    }
    return status;
}
