/**
 * @file
 * @brief Separate version discovery work queues, entry checks and selection publication.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps physical
 * discovery separate from parsing. [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * requires version-level selection before ordinary module scanning.
 */
#ifndef AMA_SOURCE_MODULE_PACKAGES_INTERNAL_H
#define AMA_SOURCE_MODULE_PACKAGES_INTERNAL_H

#include "reader.h"
#include "../internal.h"

/** Arena-owned unversioned namespace directory awaiting one finite scan.
 * Depth counts vendor/package components, not physical storage prefixes.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) makes complete version
 * components terminal boundaries rather than further namespace directories.
 */
typedef struct PackageDirectory {
    struct PackageDirectory *next;
    const char *physical;
    const char *relative;
    const char *logical;
    size_t depth;
} PackageDirectory;

/** One storage scan borrows its root and arena-backed selected list/queue.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) supplies local before
 * external pass ordering; no module, configuration or companion is read here.
 */
typedef struct {
    ModuleCatalog *catalog;
    const ModulePackageReader *reader;
    const ModuleRoot *root;
    ModulePackage **selected;
    PackageDirectory *pending;
    PackageDirectory *last;
    bool local;
} PackageScan;

/** Queue one exact directory after global physical identity registration.
 * Strings are arena-owned; failure retains only owned scratch and diagnostic
 * context. [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * rejects aliases rather than revisiting a physical directory under another name.
 */
ModuleStatus package_queue(PackageScan *, const char *, const char *, const char *, size_t, const struct stat *);

/** Read one optional storage tree iteratively, stopping at every version boundary.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) permits absent storage;
 * a supplied non-directory or failed read is not silently treated as absence.
 */
ModuleStatus package_discover(PackageScan *);

/** Classify one exact entry and either queue a namespace or select a version.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) excludes version-named
 * vendor/package components. Non-directory files outside versions are not
 * module candidates; [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * preserves exact physical probing for actual candidates.
 */
ModuleStatus package_visit(PackageScan *, const PackageDirectory *, const char *);

/** Check whether an already selected local version makes this external candidate irrelevant.
 * No physical contents are read. [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * requires ignored copies to contribute no modules or metadata.
 */
bool package_ignored(const PackageScan *, const char *);

/** Insert a checked version directory in logical order, refusing same-priority duplicates.
 * All candidate fields are arena-owned except the build-borrowed root.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) forbids composing one
 * version from competing configured roots or choosing by enumeration order.
 */
ModuleStatus package_register(PackageScan *, const char *, const char *, const char *);

#endif
