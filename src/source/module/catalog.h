/**
 * @file
 * @brief Own one exact physical-source catalog and its diagnostic lifetimes.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Select project/import or standard roots explicitly; complete multiword components
 * preserve original bytes and never shadow an earlier physical source.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) makes the declared
 * root kind authoritative instead of inferring it from a physical directory name.
 */
#ifndef AMA_SOURCE_MODULE_CATALOG_H
#define AMA_SOURCE_MODULE_CATALOG_H

#include <sys/types.h>
#include "source/memory/arena.h"
#include "source/text/source.h"

/** Authoritative source identity/bytes captured by ordinary discovery; snapshot construction deep-copies text and names. */
typedef struct {
    const char *logical_path;
    const SourceText *text;
    dev_t device;
    ino_t inode;
} ModuleSnapshotEntry;

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
    bool native_loaded;
} ModuleSource;

/** Logical namespace family; zero is deliberately invalid until a root is explicitly configured. */
typedef enum {
    MODULE_ROOT_PROJECT = 1,
    MODULE_ROOT_STANDARD = 2
} ModuleRootKind;

/** Borrowed physical root plus exact relative exclusions and required namespace family.
 * Project/import roots share the single-dot family; standard roots use double dot.
 * Physical directory names do not determine or override the declared kind.
 */
typedef struct {
    const char *path;
    /* Exact physical paths relative to this root, including output subtrees.
     * No guessed exclusions based on otherwise legal module names. */
    const char *const *excluded;
    size_t excluded_count;
    ModuleRootKind kind;
} ModuleRoot;

/** Borrowed catalog-owned failure context, including both conflict paths or an original malformed-byte source. */
typedef struct {
    ModuleStatus status;
    const char *path;
    const char *related_path;
    const SourceText *source;
    size_t offset;
} ModuleIssue;

/**
 * Opaque catalog-owned physical directory identity used to reject aliases during iterative
 * discovery.
 */
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
/** Build once from already discovered authoritative source views without rereading Ama files.
 * Validate exact names, UTF-8 and logical/physical uniqueness; copied records/bytes belong to
 * this catalog. Native companions remain selected by the ordinary later activation path.
 */
ModuleStatus module_catalog_snapshot(ModuleCatalog *, const ModuleSnapshotEntry *, size_t);
/** Install authoritative captured native companions before AST consumers exist.
 * Deep-copy optional paths and ABI bytes without filesystem access. The selected record
 * must belong to this complete catalog and have no loaded native companions. Success
 * seals even an empty companion set; failure preserves all previous companion fields.
 * Caller establishes native activation and captured companion authority before calling.
 */
ModuleStatus module_catalog_snapshot_native(ModuleCatalog *, const ModuleSource *,
    const char *body, const char *header, const SourceText *abi);
/** Look up one validated retained logical spelling after successful build.
 * @param[out] output Changes only on MODULE_OK and borrows catalog lifetime.
 * @return Invalid path/partial build, absent module or exact match; no root precedence is applied.
 * @param[in] catalog Borrowed complete catalog retaining every source record through output use.
 * @param[in] logical_path Borrowed exact full logical identity; no prefix, normalization or fallback lookup.
 */
ModuleStatus module_catalog_find(const ModuleCatalog *catalog,
    const char *logical_path, const ModuleSource **output);
/** Load optional physical companions for a catalog-owned source after native activation is established.
 * Missing companions remain null for explicit authoring; strict native validation is a later stage.
 * Successful repeated requests reuse the same views. Failure publishes no partial companion set;
 * catalog-owned scratch and original failure locations survive until destruction.
 * @return MODULE_OK, invalid catalog/member, allocation, exact-path, IO or UTF-8 failure.
 */
ModuleStatus module_catalog_load_native(ModuleCatalog *, const ModuleSource *);
/** Install an explicit copied candidate text in a complete owned catalog before any AST/native
 * consumer exists. Logical/physical identity is preserved; UTF-8/allocation failure leaves the
 * selected source unchanged and retains diagnostic context. Other records stay unchanged.
 */
ModuleStatus module_catalog_override(ModuleCatalog *, const ModuleSource *, const unsigned char *, size_t);
/** Read exact sorted non-dot directory entry names into catalog-owned scratch without
 * classifying them or following links. Usable before/after build; existing records stay stable.
 * physical is borrowed and must outlive any retained failure issue. On failure names is
 * unchanged, count may describe a copied prefix; all scratch survives until destroy.
 */
ModuleStatus module_catalog_entries(ModuleCatalog *, const char *physical, const char ***names, size_t *count);
/** Validate an absolute single/double-dot selector and nonempty complete-name components.
 * @return Exact spelling membership with no edge/separator spaces, filesystem parent traversal or normalization.
 */
bool module_path_valid(const char *path);

#endif
