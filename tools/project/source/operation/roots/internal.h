/** @file
 * @brief Separate spatial coverage, original identity preservation and explicit setting accounting.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not turn a root-setting decision into a physical write grant.
 */
#ifndef AMA_SOURCE_OPERATION_ROOTS_INTERNAL_H
#define AMA_SOURCE_OPERATION_ROOTS_INTERNAL_H
#include "roots.h"
#include "../../mapping/identity.h"
/** Decide one symbolic root region's inclusion in a union. Descriptors have
 * canonical absolute paths and canonical nonempty relative exclusions, as in
 * admitted captures; they need not describe existing host directories. Return
 * the first uncovered boundary or NULL on success. Scratch is arena-owned;
 * no owner flags, I/O, finite-file sampling or filesystem claims are involved. */
SourceOperationRootsStatus source_operation_root_region(SourceArena *, const ModuleRoot *,
    const ModuleRoot *, size_t, const char **failed_path);
/** Exact union-of-root-regions coverage, including every exclusion boundary. */
SourceOperationRootsStatus source_operation_roots_coverage(SourceOperationRoots *);
/** Preserve configured identity/material before common augmented-original analysis. */
SourceOperationRootsStatus source_operation_roots_originals(SourceOperationRoots *);
/** Account for each explicit original/final setting and actual carried placement. */
SourceOperationRootsStatus source_operation_roots_settings(SourceOperationRoots *);
#endif
