/** @file
 * @brief Compute the specified physical placement of an explicit raw SOURCE tree.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * fixes directory-relative and module-stem placement. Logical identity discovery,
 * added-unit maps, replacement and publication remain separate obligations.
 */
#ifndef AMA_PROJECT_SOURCE_TREE_PLACEMENT_H
#define AMA_PROJECT_SOURCE_TREE_PLACEMENT_H
#include "../selection/domain.h"
/** Keep the exact captured original bytes and one owned proposed final physical key. */
typedef struct SourceTreePlacementEntry {
    struct SourceTreePlacementEntry *next;
    const AcquisitionUnitEntry *original;
    const char *path;
} SourceTreePlacementEntry;
/** Placement construction is not permission to publish over an existing target. */
typedef enum { SOURCE_PLACEMENT_OK, SOURCE_PLACEMENT_INVALID,
    SOURCE_PLACEMENT_DOMAIN, SOURCE_PLACEMENT_MEMORY } SourcePlacementStatus;
/** Own exact selected-tree proposals, borrowing its complete original captures.
 * ready certifies this physical translation only. No destination existence,
 * aliases, logical-name correspondence or final graph facts are inferred.
 */
typedef struct {
    SourceArena arena;
    const SourceTreeSelection *selection;
    const char *destination;
    SourceTreePlacementEntry *entries;
    SourceTreeDomainStatus domain_status;
    const char *failed_path;
    SourcePlacementStatus status;
    bool attempted, ready;
} SourceTreePlacement;
/** Begin owned placement preparation. */
void source_tree_placement_init(SourceTreePlacement *, SourceAllocator);
/** Retire physical proposals before the borrowed raw selection expires. */
void source_tree_placement_destroy(SourceTreePlacement *);
/** Translate a complete admitted raw selection into a previously normalized
 * absolute destination key. Directory operands preserve every relative component;
 * module operands rename only the main stem, its dot companions and plus subtree.
 * Destination is the final location itself. Prepare once; only ready permits
 * consumption. Refusal leaves owned scratch and no source mutation. Caller must
 * separately reject overlap/existing targets, validate roots, derive actual final
 * identities and satisfy all additional map/graph/transaction requirements.
 */
SourcePlacementStatus source_tree_placement_build(SourceTreePlacement *, const SourceTreeSelection *, const char *destination);
#endif
