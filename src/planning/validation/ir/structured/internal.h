/** @file
 * @brief Share exact owner-inventory checks for lexical loops and initiating operation trees.
 * [COMP-02](../../../../../spec/implementation/compiler.md#comp-02) requires
 * complete resolved identities and explicit successors before publication.
 */
#ifndef AMA_IR_STRUCTURED_INTERNAL_H
#define AMA_IR_STRUCTURED_INTERNAL_H
#include "../internal.h"
#include "planning/storage/frames/internal.h"
/** Test a control endpoint without following executable backedges. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02). */
static inline bool structured_node(const PlannedType *type, const ControlNode *candidate) {
    for (const ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node == candidate) { return true; }
    }
    return false;
}
/** Find one exact source loop identity in this type's fixed inventory. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02). */
const PlannedLoop *structured_loop(const PlannedType *, const SemanticStatement *);
/** Test chain ownership before dereferencing an instruction's chain operand. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02). */
bool structured_chain(const PlannedType *, const PlannedChain *);
/** Validate loop labels, lexical containment and traversal resume templates. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02). */
bool structured_loops(const PlannedType *);
/** Validate root/exit/handler identities for every initiating source operation. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02). */
bool structured_chains(const PlannedType *);
#endif
