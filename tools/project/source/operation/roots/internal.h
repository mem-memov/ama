/** @file
 * @brief Separate spatial coverage, original identity preservation and explicit setting accounting.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not turn a root-setting decision into a physical write grant.
 */
#ifndef AMA_SOURCE_OPERATION_ROOTS_INTERNAL_H
#define AMA_SOURCE_OPERATION_ROOTS_INTERNAL_H
#include "region.h"
#include "../../mapping/identity.h"
/** Exact union-of-root-regions coverage, including every exclusion boundary. */
SourceOperationRootsStatus source_operation_roots_coverage(SourceOperationRoots *);
/** Preserve configured identity/material before common augmented-original analysis. */
SourceOperationRootsStatus source_operation_roots_originals(SourceOperationRoots *);
/** Account for each explicit original/final setting and actual carried placement. */
SourceOperationRootsStatus source_operation_roots_settings(SourceOperationRoots *);
#endif
