/** @file
 * @brief Separate explicit raw placement from exact original removal-union review.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * forbids self-overlap and implicit removal of dependency originals.
 */
#ifndef AMA_SOURCE_OPERATION_INTENT_INTERNAL_H
#define AMA_SOURCE_OPERATION_INTENT_INTERNAL_H
#include "intent.h"
/** Compare every explicit raw placement with P and reject source/output overlap. */
SourceOperationIntentStatus source_operation_intent_placement(const SourceOperationPlan *,
    const SourceOperationIntent *, SourceOperationIntentIssue *);
/** Account for the complete physical/name union of source and replacement cuts. */
SourceOperationIntentStatus source_operation_intent_removal(const SourceOperationPlan *,
    const SourceOperationIntent *, SourceOperationIntentIssue *);
/** Compare actual path/name sets with the exact union of admitted contribution owners. */
SourceOperationIntentStatus source_operation_intent_union(const SourceOperationPlan *,
    const SourceOperationIntent *, SourceOperationIntentIssue *);
#endif
