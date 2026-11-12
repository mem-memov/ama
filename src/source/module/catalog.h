/**
 * @file
 * @brief Own one exact physical-source catalog and its diagnostic lifetimes.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Build without root shadowing; the current single-dot/word path representation awaits T016.
 */
#ifndef AMA_SOURCE_MODULE_CATALOG_H
#define AMA_SOURCE_MODULE_CATALOG_H

#include <sys/types.h>
#include "source/memory/arena.h"
#include "source/text/source.h"

/** Catalog outcomes distinguish absence, aliases, unsupported physical storage and malformed UTF-8. */
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

/** Catalog-owned source/companion record; original device/inode support exact alias detection. */
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

/** Borrowed physical root plus exact relative exclusions, used only during build. */
typedef struct {
    const char *path;
    /* Exact physical paths relative to this root, including output subtrees.
     * No guessed exclusions based on otherwise legal module names. */
    const char *const *excluded;
    size_t excluded_count;
} ModuleRoot;

/** Borrowed catalog-owned failure context, including both conflict paths or an original malformed-byte source. */
typedef struct {
    ModuleStatus status;
    const char *path;
    const char *related_path;
    const SourceText *source;
    size_t offset;
} ModuleIssue;

typedef struct ModuleDirectoryIdentity ModuleDirectoryIdentity;

/** Build-once owner of records, source bytes, directory identities and failure context. */
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
/** Initialize empty owned source/arena registries with borrowed allocator authority.
 */
void module_catalog_init(ModuleCatalog *catalog, SourceAllocator allocator);
/** Release complete or failed-build storage. All catalog views expire; initialize again before another build.
 */
void module_catalog_destroy(ModuleCatalog *catalog);
/** Build once over borrowed stable roots/exclusions. Publish built only after all roots succeed; partial allocations/issue storage remain owned until destroy.
 */
ModuleStatus module_catalog_build(ModuleCatalog *catalog,
    const ModuleRoot *roots, size_t root_count);
/** Look up one validated retained logical spelling after successful build.
 * @param[out] output Changes only on MODULE_OK and borrows catalog lifetime.
 * @return Invalid path/partial build, absent module or exact match; no root precedence is applied.
 */
ModuleStatus module_catalog_find(const ModuleCatalog *catalog,
    const char *logical_path, const ModuleSource **output);
/** Validate the retained single-dot, nonempty Unicode-word component form.
 * @return Exact supported-form membership only; complete multiword names and distinct standard-library roots remain T016.
 */
bool module_path_valid(const char *path);

#endif
