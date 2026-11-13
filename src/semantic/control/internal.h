/** @file
 * @brief Separate deferred predicate selection from lexical traversal-role completion.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) records the full
 * restart root; [FLOW-05](../../../spec/language/lists.md#flow-05) reuses exact
 * source-order iterator bindings without creating runtime cells during analysis.
 */
#ifndef AMA_SEMANTIC_CONTROL_INTERNAL_H
#define AMA_SEMANTIC_CONTROL_INTERNAL_H
#include "semantic/internal.h"
/** Resolve a caller or parent-result terminal predicate, requiring its own native condition capability and recording the complete restart root. */
bool semantic_control_condition(SemanticProgram *, SemanticType *, SemanticStatement *);
/** Link a traversal to its exact previously typed collection/binder role; missing metadata is an explicit invalid-program failure. */
bool semantic_control_traversal(SemanticProgram *, SemanticType *, SemanticStatement *);
#endif
