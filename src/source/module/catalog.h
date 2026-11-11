#ifndef AMA_SOURCE_MODULE_CATALOG_H
#define AMA_SOURCE_MODULE_CATALOG_H

#include <sys/types.h>
#include "source/memory/arena.h"
#include "source/text/source.h"

typedef enum {
    MODULE_OK,
    MODULE_MISSING,
    MODULE_OUT_OF_MEMORY,
    MODULE_IO_ERROR,
    MODULE_INVALID_PATH,
    MODULE_UNSUPPORTED_PATH,
    MODULE_ORPHAN_DIRECTORY,
    MODULE_AMBIGUOUS,
    MODULE_FILE_ALIAS,
    MODULE_INVALID_UTF8
} ModuleStatus;

typedef struct ModuleSource {
    struct ModuleSource *next;
    const char *logical_path;
    const SourceText *text;
    const char *native_c;
    const char *native_header;
    const SourceText *native_abi;
    dev_t device;
    ino_t inode;
} ModuleSource;

typedef struct {
    const char *path;
    /* Exact physical paths relative to this root, including output subtrees.
     * No guessed exclusions based on otherwise legal module names. */
    const char *const *excluded;
    size_t excluded_count;
} ModuleRoot;

typedef struct {
    ModuleStatus status;
    const char *path;
    const char *related_path;
    const SourceText *source;
    size_t offset;
} ModuleIssue;

typedef struct ModuleDirectoryIdentity ModuleDirectoryIdentity;

typedef struct {
    SourceArena arena;
    ModuleDirectoryIdentity *directories;
    SourceManager sources;
    ModuleSource *first;
    ModuleIssue issue;
    bool built;
    bool attempted;
} ModuleCatalog;

/* Build once, inspect until destroy. All source/record/path views are owned by
 * the catalog, including failure details. No lookup on a failed partial build.
 * Roots/exclusions are borrowed for the build call only. Inputs must be stable. */
void module_catalog_init(ModuleCatalog *catalog, SourceAllocator allocator);
void module_catalog_destroy(ModuleCatalog *catalog);
ModuleStatus module_catalog_build(ModuleCatalog *catalog,
    const ModuleRoot *roots, size_t root_count);
ModuleStatus module_catalog_find(const ModuleCatalog *catalog,
    const char *logical_path, const ModuleSource **output);
bool module_path_valid(const char *path);

#endif
