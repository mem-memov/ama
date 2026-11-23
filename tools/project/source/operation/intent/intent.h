/** @file
 * @brief Prove the requested source operation's physical placement and original removal provenance.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * separates copy, deletion and move-source removal and requires explicit safe
 * replacement. [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * reviews their exact original union, never a dependency-cloning closure.
 */
#ifndef AMA_SOURCE_OPERATION_INTENT_H
#define AMA_SOURCE_OPERATION_INTENT_H
#include "../plan/plan.h"
/** Move copies selected material and removes its original; extra dependencies
 * remain originals. Deletion introduces no copies or replacement permission. */
typedef enum { SOURCE_OPERATION_COPY, SOURCE_OPERATION_DELETE, SOURCE_OPERATION_MOVE } SourceOperationKind;
/** Borrow genuine immutable source/placement and removal owners through review.
 * source_removal is present exactly for DELETE/MOVE and must belong to source.
 * replacements are complete existing destination selections, not arbitrary
 * source cuts. replace grants permission to review them, never to waive safety.
 */
typedef struct {
    SourceOperationKind kind;
    const SourceTreeSelection *source;
    const SourceTreePlacement *placement;
    const SourceRemovalSelection *source_removal;
    const SourceRemovalSelection *const *replacements;
    size_t replacement_count;
    bool replace;
} SourceOperationIntent;
/** Preserve admission, raw mapping, overlap, replacement and union failures. */
typedef enum { SOURCE_OPERATION_INTENT_OK, SOURCE_OPERATION_INTENT_INVALID,
    SOURCE_OPERATION_INTENT_DOMAIN, SOURCE_OPERATION_INTENT_PLACEMENT,
    SOURCE_OPERATION_INTENT_OVERLAP, SOURCE_OPERATION_INTENT_REPLACE,
    SOURCE_OPERATION_INTENT_REMOVAL } SourceOperationIntentStatus;
/** Borrow an exact offending key and selection; clear on complete success. */
typedef struct {
    const char *path;
    const SourceRemovalSelection *selection;
} SourceOperationIntentIssue;
/** Check one prepared shared image against its explicit request provenance.
 * Every raw placement must occur unchanged in P; no mapped output may overlap
 * selected original material. Removed paths/identities equal exactly the union
 * of the selected source cut and authorized complete destination cuts. Each
 * replacement starts at the explicit destination or a mapped P destination,
 * preventing implicit ancestor/sibling deletion. Empty/duplicate contributions
 * use exact set union, not order-dependent ownership. Original graph/client cut,
 * observed material and final configuration must already pass plan preparation.
 *
 * No allocation, I/O, execution or mutation. With N mapped files, K raw
 * placements, R replacement owners and F total actual/contributed removal keys,
 * finite scans take O(NK + RN + F squared) key/byte comparisons and O(1) scratch.
 * Output issue resets on entry. Success proves physical request provenance only;
 * full material completeness, extra-write/root/client authorization, final
 * compiler proof and recoverable publication remain separate mandatory gates.
 */
SourceOperationIntentStatus source_operation_intent_check(const SourceOperationPlan *,
    const SourceOperationIntent *, SourceOperationIntentIssue *);
#endif
