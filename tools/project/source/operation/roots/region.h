/** @file
 * @brief Decide symbolic root-region inclusion without filesystem sampling or namespace inference.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires complete original coverage, including newly exposed exclusion regions.
 */
#ifndef AMA_SOURCE_OPERATION_ROOT_REGION_H
#define AMA_SOURCE_OPERATION_ROOT_REGION_H
#include "roots.h"
/** Decide one root region's inclusion in a finite union. Descriptors have
 * canonical absolute paths and canonical nonempty relative exclusions, as in
 * admitted captures; they need not describe existing host directories. Return
 * the first uncovered boundary or NULL on success. Scratch is arena-owned;
 * no owner flags, I/O, finite-file sampling or filesystem claims are involved.
 * The finite root/exclusion endpoint partition is exact: membership is constant
 * between adjacent subtree boundaries. Child spelling/boundary scans determine
 * time and scratch; no recursion, arbitrary depth cap or source mutation occurs.
 */
SourceOperationRootsStatus source_operation_root_region(SourceArena *, const ModuleRoot *,
    const ModuleRoot *, size_t, const char **failed_path);
#endif
