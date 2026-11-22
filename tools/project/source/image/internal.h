/** @file
 * @brief Share exact immutable-image lookup and catalog-reader callbacks.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * uses an explicit prepared view; no physical source probe may fill its holes.
 */
#ifndef AMA_PROJECT_SOURCE_CATALOG_IMAGE_INTERNAL_H
#define AMA_PROJECT_SOURCE_CATALOG_IMAGE_INTERNAL_H
#include "image.h"
/** Match an exact bounded physical key; rows are immutable through consumption. */
const SourceCatalogImageEntry *source_image_find(const SourceCatalogImage *, const char *, size_t);
/** Exact directory verification below an explicit root, preserving typed failures. */
ModuleStatus source_image_root(void *, ModuleCatalog *, const char *, struct stat *);
/** Exact child probe; only a complete parent listing may establish absence. */
ModuleStatus source_image_child(void *, ModuleCatalog *, const char *, const char *, const char *, struct stat *);
/** Supply complete sorted immediate names in the destination catalog's arena. */
ModuleStatus source_image_entries(void *, ModuleCatalog *, const char *, const char ***, size_t *);
/** Borrow captured file bytes only on a fully observed regular entry. */
SourceStatus source_image_read(void *, const char *, const SourceText **);
#endif
