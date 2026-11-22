/** @file
 * @brief Compute the exact finite image of original explicit dependency roots.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains unchanged selections and maps copied configuration selections without
 * expanding original wildcard matches in an augmented destination domain.
 */
#ifndef AMA_SOURCE_COPY_SELECTED_ROOTS_H
#define AMA_SOURCE_COPY_SELECTED_ROOTS_H
#include "../../mapping/map.h"
#include "source/configuration/selection/selection.h"
/** Actual version endpoints; outer whole-version placement supplies authority. */
typedef struct { const DependencyViewEntry *original, *final; } SourceSelectionVersionPair;
/** Array is arena-owned; immutable entries borrow the final view. */
typedef struct { const DependencyViewEntry *const *entries; size_t count; } SourceSelectionRoots;
/** Conflicting version demands are separate from malformed owner/category inputs. */
typedef enum { SOURCE_SELECTION_ROOTS_OK, SOURCE_SELECTION_ROOTS_INVALID,
    SOURCE_SELECTION_ROOTS_CONFLICT, SOURCE_SELECTION_ROOTS_LIMIT,
    SOURCE_SELECTION_ROOTS_MEMORY } SourceSelectionRootsStatus;
/** Borrow the exact conflicting explicit version rows; cleared otherwise. */
typedef struct { const SourceSelectionVersionPair *first, *second; } SourceSelectionRootsIssue;
/** Require a successful original selection and actual final view. For copied
 * configurations, require ready C and actual version pairs: module roots map
 * through C; version roots map only through those explicit pairs. Unmapped roots
 * retain identity. For unchanged configurations require NULL C/no pairs.
 * Validate every pair against its actual owning view and version extent; equal
 * repeats are idempotent, unequal destinations for one original conflict.
 *
 * Return the canonical unique actual final entries whose names occur in that
 * image. Missing names contribute no match under PKG-05. This does not grant new
 * permissions, prove a whole-version placement or reinterpret unit coverage.
 * Inputs remain immutable through consumption. No I/O/allocation outside arena,
 * compiler state or mutation. Output changes only on success; partial scratch
 * remains arena-owned. Finite view/root/map/pair scans and one output array bound
 * preparation. Final reparse, exact root-set/edge laws and transactions remain
 * mandatory outer gates.
 */
SourceSelectionRootsStatus source_copy_selected_roots(SourceArena *, const DependencySelection *,
    const DependencyView *, const SourceCloneMap *, bool copied,
    const SourceSelectionVersionPair *, size_t, SourceSelectionRoots *, SourceSelectionRootsIssue *);
#endif
