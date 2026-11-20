/**
 * @file
 * @brief Select coherent physical package-version copies before indexing their modules.
 *
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) applies local precedence
 * to an entire version, so module discovery must consume this selection rather
 * than resolve competing files independently.
 */
#ifndef AMA_SOURCE_MODULE_PACKAGES_H
#define AMA_SOURCE_MODULE_PACKAGES_H

#include "source/module/catalog.h"

/** Catalog-arena-owned selected version; root is borrowed only during build.
 * Other strings and links remain arena-owned until catalog destruction. This
 * preparation record is consumed before completing catalog or projected-preview
 * discovery; neither operation retains the borrowed root beyond its build.
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) supplies the exact logical
 * version prefix; [PKG-02](../../../../spec/project/packages.md#pkg-02) selects
 * one physical version, never individual files from several versions.
 */
typedef struct ModulePackage {
    struct ModulePackage *next;
    const ModuleRoot *root;
    const char *physical;
    const char *relative;
    const char *logical;
    bool local;
} ModulePackage;

/** Discover/select version directories without reading their module contents.
 * @param[in,out] catalog Fresh initialized pre-index owner; all partial storage and
 * failure context remain owned until destroy. No earlier discovery or source
 * indexing may have populated its directory/source records.
 * @param[in] roots Borrowed stable root descriptors through selection/indexing;
 * ordinary descriptors identify project roots containing optional storage trees.
 * Standard descriptors contribute no ordinary versions.
 * @param[in] count Number of roots; zero permits NULL roots.
 * @param[out] output Changes only on success to a logical-name-sorted arena list.
 * @return Exact shape/alias/duplicate/I/O/allocation outcome. No source is loaded,
 * no fallback copy is indexed, and no filesystem contents are modified.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) requires all local
 * candidates to be selected before considering external copies, independent
 * of root order. [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * retains exact spelling and physical alias diagnostics.
 */
ModuleStatus module_packages_select(ModuleCatalog *catalog, const ModuleRoot *roots,
    size_t count, ModulePackage **output);

/** Index the required areas of one already selected version, preserving exclusions.
 * @param[in,out] catalog Build owner of source bytes and complete/failed scratch.
 * @param[in] package Arena-owned selection with a root descriptor borrowed through build.
 * @return Missing/shape/alias/I/O/allocation or source outcome. A missing local
 * input never triggers another copy's lookup. [PKG-01](../../../../spec/project/packages.md#pkg-01)
 * requires package.ini/src/tests; [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * makes this selected package the sole authority for both source and test modules.
 */
ModuleStatus module_package_index(ModuleCatalog *catalog, const ModulePackage *package);

#endif
