/** @file
 * @brief Separate exact image-delta recognition from complete mutation material preparation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires both complete old trees and verified new trees before publication.
 */
#ifndef AMA_SOURCE_TRANSACTION_FOREST_INTERNAL_H
#define AMA_SOURCE_TRANSACTION_FOREST_INTERNAL_H
#include "forest.h"
/** Exact path query over validated unique image keys; no missing-row fallback. */
const SourceCatalogImageEntry *source_forest_find(const SourceCatalogImage *, const char *);
/** Compare only observed filesystem material, not unused timestamps/stat fields. */
bool source_forest_equal(const SourceCatalogImageEntry *, const SourceCatalogImageEntry *);
/** Retain every exact subtree row in byte order, refusing unknown bytes/kinds. */
SourceForestStatus source_forest_material(SourceArena *, const SourceCatalogImage *, const char *,
    const SourceCatalogImageEntry *const **, size_t *, SourceForestIssue *);
/** Establish known complete unchanged parent and owned parent-path spelling. */
SourceForestStatus source_forest_parent(SourceArena *, const SourceCatalogImage *, const SourceCatalogImage *,
    SourceMutationRoot *, SourceForestIssue *);
#endif
