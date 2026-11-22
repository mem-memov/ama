/** @file
 * @brief Derive concrete permission additions from actual copied dependency edges.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * authorizes new concrete external targets only in EXTEND_PACKAGE mode; it never
 * permits broad masks, guessed libraries or repository-table additions.
 */
#ifndef AMA_SOURCE_COPY_PERMISSION_ADDITIONS_H
#define AMA_SOURCE_COPY_PERMISSION_ADDITIONS_H
#include "roots.h"
#include "../closure.h"
#include "../../environment/environment.h"
/** Preserve availability, policy and direction failures independently of memory. */
typedef enum { SOURCE_PERMISSION_OK, SOURCE_PERMISSION_INVALID,
    SOURCE_PERMISSION_MEMORY, SOURCE_PERMISSION_LIMIT, SOURCE_PERMISSION_MISSING,
    SOURCE_PERMISSION_UNDECLARED, SOURCE_PERMISSION_DIRECTION } SourcePermissionStatus;
/** Borrow the original source/edge and exact expected final target name. */
typedef struct {
    const SourceCloneMapping *source;
    const DeliverySourceDependency *edge;
    const char *expected;
    DependencyEdgeDecision decision;
} SourcePermissionIssue;
/** For one actual final package scope, examine admitted original definitions
 * whose C image belongs to that version. Require valid copy/final environments,
 * ready C over the copy's exact capture, and canonical unique actual baseline
 * roots (the result of source_copy_selected_roots). Baseline is fixed throughout:
 * evaluate normal shared PKG-05 coverage and PKG-09 directions before additions.
 * Target is C(target) when mapped, otherwise original; actual final availability
 * is required. Standard and same-version targets require no new selector.
 *
 * Only EXTEND_PACKAGE may collect uncovered external ordinary targets; other
 * modes refuse. Deduplicate exact targets against the fixed baseline and emit in
 * actual view order, independently of source/edge order. One added target may
 * structurally cover another; both remain explicit actual-edge permissions if
 * neither was covered by the original baseline. Never perform a greedy cover.
 *
 * Output changes only on success; failed scratch remains arena-owned. All inputs
 * remain immutable and outlive consumption. No I/O, compiler execution or source
 * mutation. Finite graph/C/view/root scans bound work and scratch. This checks
 * one scope only: caller must cover every copied source version, then reparse
 * final configurations, prove exact selected sets/edges, final graph/sites,
 * complete material, freshness and safe atomic publication.
 */
SourcePermissionStatus source_copy_permission_additions(SourceArena *, const SourceCopyClosure *,
    const SourceCloneMap *, const SourcePackageEnvironment *, const SourcePackageScope *,
    const SourceSelectionRoots *, SourceSelectionRoots *, SourcePermissionIssue *);
#endif
