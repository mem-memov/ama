/** @file
 * @brief Join same-path original/final analysis scopes by exact exclusion-set intersection.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * exposes new inputs without duplicating a covered physical root or renaming old definitions.
 */
#ifndef AMA_PROJECT_SOURCE_AUGMENTATION_SCOPES_H
#define AMA_PROJECT_SOURCE_AUGMENTATION_SCOPES_H
#include "augmentation.h"
/** Replace only target's exclusion descriptor with the exact intersection of
 * its old excluded-subtree union and added's excluded-subtree union. Paths/kinds
 * must match. Since (R minus A) union (R minus B) = R minus (A intersection B),
 * this is the exact scope union. Two excluded subtrees intersect iff their keys
 * are prefix-comparable; their intersection is the deeper key. Deduplicate equal
 * keys. Borrow key strings; own the checked at-most A*B pointer array in arena.
 * No I/O, mutation of input owners or sampled-file inference; O((AB)^2) key work.
 */
ProjectSourceAugmentationStatus project_source_augmentation_scope_union(SourceArena *, ModuleRoot *target, const ModuleRoot *added);
#endif
