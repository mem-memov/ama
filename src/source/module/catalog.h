/**
 * @file
 * @brief Own one exact physical-source catalog and its diagnostic lifetimes.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Select versioned project or standard roots explicitly; complete multiword components
 * preserve original bytes and never shadow an earlier physical source.
 * [PKG-02](../../../spec/project/packages.md#pkg-02) retains one selected version;
 * [PKG-03](../../../spec/project/packages.md#pkg-03) owns its configuration bytes.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) tracks active companions.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) makes the declared
 * root kind authoritative instead of inferring it from a physical directory name.
 */
#ifndef AMA_SOURCE_MODULE_CATALOG_H
#define AMA_SOURCE_MODULE_CATALOG_H

#include <sys/types.h>
#include <sys/stat.h>
#include "source/memory/arena.h"
#include "source/text/source.h"
#include "source/path/root.h"

/** Authoritative source identity/bytes and optional module configuration. NULL configuration
 * explicitly captures absence; snapshot construction deep-copies present text and names.
 * [PKG-03](../../../spec/project/packages.md#pkg-03) preserves optional companion authority.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
typedef struct {
    const char *logical_path;
    const SourceText *text;
    dev_t device;
    ino_t inode;
    const SourceText *configuration;
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

/** Retained selected-version identity and immutable configuration bytes, all catalog-owned.
 * Root descriptors are never retained. Empty versions have records even without modules.
 * [PKG-02](../../../spec/project/packages.md#pkg-02) selects one physical copy;
 * [PKG-03](../../../spec/project/packages.md#pkg-03) gives its configuration authority.
 */
typedef struct ModuleVersion {
    struct ModuleVersion *next;
    const char *logical_path;
    const char *physical_path;
    const SourceText *configuration;
} ModuleVersion;

/** Catalog-owned source/companion record; original device/inode support exact alias detection.
 * configuration_loaded seals both presence and absence; configuration_path retains the
 * anticipated .ini location even when configuration is NULL. [PKG-03](../../../spec/project/packages.md#pkg-03)
 * keeps the local file distinct from package configuration and native companions.
 * Component contract: [SRC-02](../../../spec/language/source-and-names.md#src-02), [SRC-01](../../../spec/language/source-and-names.md#src-01). */
typedef struct ModuleSource {
    struct ModuleSource *next;
    const char *logical_path;
    const SourceText *text;
    const ModuleVersion *version;
    const char *configuration_path;
    const SourceText *configuration;
    bool configuration_loaded;
    const char *native_c;
    const char *native_header;
    const SourceText *native_abi;
    dev_t device;
    ino_t inode;
    bool native_loaded;
} ModuleSource;

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
    ModuleVersion *versions;
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
 * this catalog. NULL configuration seals captured absence; present configuration
 * is deep-copied and UTF-8 checked without disk fallback. Native companions remain selected by the ordinary later activation path.
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
/** Capture one authoritative version configuration during catalog preparation before AST consumers.
 * Deep-copy complete ordinary version identity, physical directory and exact configuration
 * source/path; no filesystem reads or INI interpretation. Reject duplicate version owners.
 * Successful attachment connects every matching ordinary source, leaving standard and other
 * versions unchanged. Failure preserves all existing version/module links; owned scratch
 * and original UTF-8 failure locations survive until destruction. The caller establishes
 * capture authority and supplies stable inputs only for this call. The initialized
 * catalog may be under construction or complete; no borrowed AST consumer may exist.
 * [PKG-02](../../../spec/project/packages.md#pkg-02) forbids borrowing another copy;
 * [PKG-03](../../../spec/project/packages.md#pkg-03) preserves original configuration bytes.
 */
ModuleStatus module_catalog_capture_version(ModuleCatalog *, const char *version,
    const char *physical_directory, const SourceText *configuration);
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
/** Capture one required module's optional .ini companion, including explicit absence.
 * Only complete catalog members are accepted. Repeated successful requests reuse
 * immutable views without I/O. Failure preserves all previous companion fields;
 * failed text/path scratch remains catalog-owned for diagnostics and destruction.
 * Source snapshots already seal captured absence/presence and never probe disk.
 * [PKG-03](../../../spec/project/packages.md#pkg-03) fixes the physical companion;
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) tracks contributing inputs.
 */
ModuleStatus module_catalog_load_configuration(ModuleCatalog *, const ModuleSource *);
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
