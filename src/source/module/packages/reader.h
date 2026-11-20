/**
 * @file
 * @brief Supply explicit read-only filesystem operations to the package selector.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) has one version selection
 * contract for compilation and projected [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * relocation views; their physical access operations differ, not precedence.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) requires each
 * reader to supply exact spelling and stable metadata for the shared alias check.
 */
#ifndef AMA_SOURCE_MODULE_PACKAGE_READER_H
#define AMA_SOURCE_MODULE_PACKAGE_READER_H
#include <sys/stat.h>
#include "packages.h"

/** Borrowed synchronous reader. Root verifies every physical path component;
 * child verifies the exact entry spelling; entries returns a sorted complete
 * finite directory view. Successful metadata includes stable device/inode.
 * Callbacks allocate returned paths/names/errors in the destination catalog or
 * a longer-lived context, never mutate source storage, and return a ModuleStatus.
 * Failure must preserve its diagnostic in catalog->issue. No callback/context
 * is retained after selection. [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * requires identical spelling/alias/error guarantees in both filesystem views.
 */
typedef struct {
    void *context;
    ModuleStatus (*root)(void *, ModuleCatalog *, const char *, struct stat *);
    ModuleStatus (*child)(void *, ModuleCatalog *, const char *, const char *, const char *, struct stat *);
    ModuleStatus (*entries)(void *, ModuleCatalog *, const char *, const char ***, size_t *);
} ModulePackageReader;

/** Select complete versions through one explicit reader, preserving the same
 * fresh-owner/input/output/cleanup contract as module_packages_select.
 * NULL readers or missing operations fail without publishing output.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) fixes the two complete
 * priority passes independently of which reader supplies filesystem evidence.
 */
ModuleStatus module_packages_select_view(ModuleCatalog *catalog, const ModuleRoot *roots,
    size_t count, const ModulePackageReader *reader, ModulePackage **output);
#endif
