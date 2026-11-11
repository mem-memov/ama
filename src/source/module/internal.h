#ifndef AMA_SOURCE_MODULE_INTERNAL_H
#define AMA_SOURCE_MODULE_INTERNAL_H

#include "catalog.h"
#include "source/path/exact.h"

struct ModuleDirectoryIdentity {
    ModuleDirectoryIdentity *next;
    const char *path;
    dev_t device;
    ino_t inode;
};

ModuleStatus module_remember_directory(ModuleCatalog *catalog, const char *path);

typedef struct ModuleDirectory {
    struct ModuleDirectory *next;
    const char *physical;
    const char *relative;
    const char *logical;
} ModuleDirectory;

typedef struct {
    ModuleCatalog *catalog;
    const ModuleRoot *root;
    ModuleDirectory *pending;
    ModuleDirectory *last;
} ModuleWalk;

ModuleStatus module_fail(ModuleCatalog *catalog, ModuleStatus status, const char *path);
ModuleStatus module_probe(ModuleCatalog *catalog, const char *parent,
    const char *name, const char *full, struct stat *metadata);
ModuleStatus module_verify_root(ModuleCatalog *catalog, const char *path);
ModuleStatus module_walk_root(ModuleCatalog *catalog, const ModuleRoot *root);
ModuleStatus module_read_entries(ModuleWalk *walk, const ModuleDirectory *directory,
    const char ***names, size_t *count);
ModuleStatus module_visit_entry(ModuleWalk *walk, const ModuleDirectory *directory,
    const char *name);
ModuleStatus module_add_source(ModuleCatalog *catalog, const char *physical,
    const char *logical, const struct stat *metadata);
ModuleStatus module_load_utf8(ModuleCatalog *catalog, const char *physical,
    const SourceText **output);
ModuleStatus module_queue(ModuleWalk *walk, const char *physical,
    const char *relative, const char *logical);

#endif
