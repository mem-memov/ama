/** @file
 * @brief Prepare a rooted fallback for one admitted copied-module dependency occurrence.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * maps targets simultaneously, preserves normalized constant members and never
 * treats implicit requiring constructs as writable path operands.
 */
#ifndef AMA_PROJECT_SOURCE_OPERANDS_ROOTED_H
#define AMA_PROJECT_SOURCE_OPERANDS_ROOTED_H
#include "../mapping/map.h"
#include "../edits/edits.h"
#include "delivery/source/records.h"
/** Fixed implicit targets are unexpressible edits, distinct from malformed handoffs. */
typedef enum { SOURCE_OPERAND_OK, SOURCE_OPERAND_INVALID, SOURCE_OPERAND_FIXED,
    SOURCE_OPERAND_EDITS } SourceOperandStatus;
/** Borrow original edge/expected module identity; retain the precise edit refusal. */
typedef struct {
    const DeliverySourceDependency *edge;
    const char *expected;
    SourceEditsStatus edits_status;
} SourceOperandIssue;
/** Add a fallback for one checked original edge of a copied definition, using
 * one sealed clone map. The edit owner borrows the exact original-export source
 * handle. Expected target is C(target) when mapped, otherwise the original target;
 * append the exported normalized member when present. Never split raw aliases or
 * relative text to recover a constant module prefix. The parsed operand interval
 * excludes storage suffixes; every outside byte remains unchanged.
 * Keep an unchanged implicit target without editing its construct; refuse a
 * changed fixed target. An already identical explicit rooted operand needs no
 * edit. Other edits go through exact overlap/ownership checks. Failure poisons
 * unattempted edits so no successful prefix can be applied. No parsing, lookup,
 * configuration mutation or filesystem access. Final alias/configuration/permission
 * validation and ordinary final compilation remain outer responsibilities.
 * Time O(C+E) key/range comparisons plus operand bytes; O(operand) scratch/new
 * replacement storage, for C clones and E prior edits. All source/map/edge owners
 * outlive edits and issue consumption. No further edits after application.
 */
SourceOperandStatus source_operand_rooted(SourceTextEdits *, const SourceCloneMap *,
    const DeliverySourceDependency *, SourceOperandIssue *);
#endif
