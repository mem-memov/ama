/**
 * @file
 * @brief Own one exact physical-source catalog and its diagnostic lifetimes.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Select versioned project or standard roots explicitly; complete multiword components
 * preserve original bytes and never shadow an earlier physical source.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) makes the declared
 * root kind authoritative instead of inferring it from a physical directory name.
 */
#ifndef AMA_SOURCE_MODULE_CATALOG_H
#define AMA_SOURCE_MODULE_CATALOG_H

#include <sys/types.h>
#include <sys/stat.h>
#include "source/memory/arena.h"
#include "source/text/source.h"

/** Authoritative source identity/bytes captured by ordinary discovery; snapshot construction deep-copies text and names. Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
typedef struct {
    const char *logical_path;
    const SourceText *text;
    dev_t device;
    ino_t inode;
} ModuleSnapshotEntry;

/** Catalog outcomes distinguish absence, aliases, unsupported physical storage and malformed UTF-8. Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
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

/** Catalog-owned source/companion record; original device/inode support exact alias detection. Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
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

/** Logical namespace family; zero is deliberately invalid until a root is explicitly configured. Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
typedef enum {
    MODULE_ROOT_PROJECT = 1,
    MODULE_ROOT_STANDARD = 2
} ModuleRootKind;

/** Borrowed physical root plus exact relative exclusions and required namespace family.
 * Project roots contain optional packages/dependencies storage and use ordinary
 * slash identities; standard roots directly contain double-slash module trees.
 * Physical directory names do not determine or override the declared kind.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
typedef struct {
    const char *path;
    /* Exact physical paths relative to this root, including output subtrees.
     * No guessed exclusions based on otherwise legal module names. */
    const char *const *excluded;
    size_t excluded_count;
    ModuleRootKind kind;
} ModuleRoot;

/** Borrowed catalog-owned failure context, including both conflict paths or an original malformed-byte source. Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
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
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
typedef struct ModuleDirectoryIdentity ModuleDirectoryIdentity;

/** Build-once owner of records, source bytes, directory identities and failure context. Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
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
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
void module_catalog_init(ModuleCatalog *catalog, SourceAllocator allocator);
/** Release complete or failed-build storage. All catalog views expire; initialize again before another build.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
void module_catalog_destroy(ModuleCatalog *catalog);
/** Build once over borrowed stable roots/exclusions. Publish built only after all roots succeed; partial allocations/issue storage remain owned until destroy.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_build(ModuleCatalog *catalog,
    const ModuleRoot *roots, size_t root_count);
/** Build once from already discovered authoritative source views without rereading Ama files.
 * Validate exact names, UTF-8 and logical/physical uniqueness; copied records/bytes belong to
 * this catalog. Native companions remain selected by the ordinary later activation path.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_snapshot(ModuleCatalog *, const ModuleSnapshotEntry *, size_t);
/** Install authoritative captured native companions before AST consumers exist.
 * Deep-copy optional paths and ABI bytes without filesystem access. The selected record
 * must belong to this complete catalog and have no loaded native companions. Success
 * seals even an empty companion set; failure preserves all previous companion fields.
 * Caller establishes native activation and captured companion authority before calling.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_snapshot_native(ModuleCatalog *, const ModuleSource *,
    const char *body, const char *header, const SourceText *abi);
/** Look up one validated retained logical spelling after successful build.
 * @param[out] output Changes only on MODULE_OK and borrows catalog lifetime.
 * @return Invalid path/partial build, absent module or exact match; no root precedence is applied.
 * @param[in] catalog Borrowed complete catalog retaining every source record through output use.
 * @param[in] logical_path Borrowed exact full logical identity; no prefix, normalization or fallback lookup.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_find(const ModuleCatalog *catalog,
    const char *logical_path, const ModuleSource **output);
/** Load optional physical companions for a catalog-owned source after native activation is established.
 * Missing companions remain null for explicit authoring; strict native validation is a later stage.
 * Successful repeated requests reuse the same views. Failure publishes no partial companion set;
 * catalog-owned scratch and original failure locations survive until destruction.
 * @return MODULE_OK, invalid catalog/member, allocation, exact-path, IO or UTF-8 failure.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_load_native(ModuleCatalog *, const ModuleSource *);
/** Install an explicit copied candidate text in a complete owned catalog before any AST/native
 * consumer exists. Logical/physical identity is preserved; UTF-8/allocation failure leaves the
 * selected source unchanged and retains diagnostic context. Other records stay unchanged.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_override(ModuleCatalog *, const ModuleSource *, const unsigned char *, size_t);
/** Read exact sorted non-dot directory entry names into catalog-owned scratch without
 * classifying them or following links. Usable before/after build; existing records stay stable.
 * physical is borrowed and must outlive any retained failure issue. On failure names is
 * unchanged, count may describe a copied prefix; all scratch survives until destroy.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
ModuleStatus module_catalog_entries(ModuleCatalog *, const char *physical, const char ***names, size_t *count);
/** Validate one complete canonical ordinary/standard identity with version/name shape.
 * @return Exact spelling membership; normalization of aliases/relative paths is
 * a separate caller operation. [PKG-01](../../../spec/project/packages.md#pkg-01)
 * supplies the ordinary vendor/package/version/area prefix.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
bool module_path_valid(const char *path);

/** Register checked directory metadata from an explicit filesystem view.
 * Path storage must outlive this catalog; no lookup or mutation is performed.
 * [SRC-02](../../../spec/language/source-and-names.md#src-02) requires the same
 * exact alias relation for observed and projected device/inode identities.
 */
ModuleStatus module_remember_directory_metadata(ModuleCatalog *, const char *, const struct stat *);

/** Test exact physical subtree exclusions only after package-version selection.
 * [PKG-02](../../../spec/project/packages.md#pkg-02) forbids using an excluded
 * or missing local module as a reason to choose an external version copy.
 */
bool module_path_excluded(const ModuleRoot *, const char *relative);

#endif
