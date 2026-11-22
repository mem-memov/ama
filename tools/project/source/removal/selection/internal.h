/** @file
 * @brief Separate associated test capture and exact removal material indexing.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * forbids inventing missing tests for deletion; [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * requires exact original removed definitions and declared file keys.
 */
#ifndef AMA_PROJECT_SOURCE_REMOVAL_SELECTION_INTERNAL_H
#define AMA_PROJECT_SOURCE_REMOVAL_SELECTION_INTERNAL_H
#include "selection.h"
/** One processed original main bounds physical material expansion to the catalog. */
typedef struct SourceRemovalProcessed {
    struct SourceRemovalProcessed *next;
    const SourceCapturedUnit *unit;
} SourceRemovalProcessed;
/** Check exact main presence in raw/associated/declared material without name guessing. */
bool source_removal_selection_contains_main(const SourceRemovalSelection *, const SourceCapturedUnit *);
/** Complete one physically selected unit and check every original indexed descendant. */
SourceRemovalSelectionStatus source_removal_selection_unit(SourceRemovalSelection *, const SourceCapturedUnit *);
/** Capture only actually present associated test material for one active src seed. */
SourceRemovalSelectionStatus source_removal_selection_tests(SourceRemovalSelection *, const SourceCapturedUnit *);
/** Add the explicit input groups of exactly one already physically selected owner. */
SourceRemovalSelectionStatus source_removal_selection_declared(SourceRemovalSelection *,
    const SourceDeclaredGroup *, const SourceCapturedUnit *);
/** Merge exact observed groups and derive byte-sorted unique original path/name sets. */
SourceRemovalSelectionStatus source_removal_selection_index(SourceRemovalSelection *);
/** Derive known original configuration/native/explicit input uses for all active owners. */
SourceRemovalSelectionStatus source_removal_selection_inputs(SourceRemovalSelection *, const SourceDeclaredGroup *);
#endif
