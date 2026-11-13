/**
 * @file
 * @brief Declare catalog scan, resolution and source-publication phase contracts.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Keep filesystem work in the catalog; it neither parses module bodies nor resolves semantic bindings.
 */
#ifndef AMA_SOURCE_MODULE_INTERNAL_H
#define AMA_SOURCE_MODULE_INTERNAL_H

#include "catalog.h"
#include "source/path/exact.h"

/** Catalog-owned visited device/inode identity; duplicate visitation is an alias error. */
struct ModuleDirectoryIdentity {
    ModuleDirectoryIdentity *next;
    const char *path;
    dev_t device;
    ino_t inode;
};

/** Record exact device/inode identity or fail on any earlier alias, including empty roots. Path storage must outlive catalog consumers.
 */
ModuleStatus module_remember_directory(ModuleCatalog *catalog, const char *path);

/** Queued immutable physical/relative paths and logical prefix views owned by the arena.
 * Initial prefixes are empty for project/import or one dot for standard roots;
 * appending dot plus a component produces the exact single/double-dot source path.
 */
typedef struct ModuleDirectory {
    struct ModuleDirectory *next;
    const char *physical;
    const char *relative;
    const char *logical;
} ModuleDirectory;

/** One root scan borrows its root and mutates only the explicit pending FIFO/catalog. */
typedef struct {
    ModuleCatalog *catalog;
    const ModuleRoot *root;
    ModuleDirectory *pending;
    ModuleDirectory *last;
} ModuleWalk;

/** Record an explicit status and borrowed stable failing path; existing related/source context is retained.
 */
ModuleStatus module_fail(ModuleCatalog *catalog, ModuleStatus status, const char *path);
/** Adapt exact-entry probe statuses while retaining the physical failing path. Parent and full/name correspondence are already verified.
 */
ModuleStatus module_probe(ModuleCatalog *catalog, const char *parent,
    const char *name, const char *full, struct stat *metadata);
/** Validate UTF-8 then verify every physical prefix as an exact nonsymlink directory. Catalog arena owns temporary paths even on failure.
 */
ModuleStatus module_verify_root(ModuleCatalog *catalog, const char *path);
/** Borrow one stable root/exclusion set for an iterative sorted scan. Every queued physical identity is unique; no recursive C directory walk is used.
 */
ModuleStatus module_walk_root(ModuleCatalog *catalog, const ModuleRoot *root);
/** Read/copy/sort actual entry names in exact byte order, excluding dot entries. Always close the stream.
 * @param[out] names Published only on success; catalog arena owns all copied names/partial scratch.
 * @param[out] count May describe a copied prefix on failure; never use it with absent names.
 * @param[in,out] walk Mutable catalog scan owner retaining copied entry names and first issue.
 * @param[in] directory Borrowed already verified directory selected by the finite scan queue.
 */
ModuleStatus module_read_entries(ModuleWalk *walk, const ModuleDirectory *directory,
    const char ***names, size_t *count);
/** Apply exact explicit exclusions, validate candidate spelling and queue a directory or load a regular source. Invalid noncandidate names are ignored under the explicit discovery contract.
 */
ModuleStatus module_visit_entry(ModuleWalk *walk, const ModuleDirectory *directory,
    const char *name);
/** Check logical/physical uniqueness, load source bytes, then link one complete record. Failed scratch and loaded bytes remain catalog-owned for diagnostics/destruction.
 */
ModuleStatus module_add_source(ModuleCatalog *catalog, const char *physical,
    const char *logical, const struct stat *metadata);
/** Load exact physical bytes, then validate UTF-8. On encoding failure output may point to the catalog-owned invalid source, and issue retains its first malformed byte.
 */
ModuleStatus module_load_utf8(ModuleCatalog *catalog, const char *physical,
    const SourceText **output);
/** Register directory identity before queueing its catalog-owned path views; a failed queue allocation leaves only catalog-owned scratch.
 */
ModuleStatus module_queue(ModuleWalk *walk, const char *physical,
    const char *relative, const char *logical);

#endif
