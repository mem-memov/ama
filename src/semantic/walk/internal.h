/**
 * @file
 * @brief Own source-ordered semantic visitation events with exact incoming loop context.
 * [NAME-04](../../../spec/language/declarations.md#name-04) includes every
 * explicit declaration; [FLOW-04](../../../spec/language/control-flow.md#flow-04)
 * keeps header handlers outside the loop whose condition was refused.
 */
#ifndef AMA_SEMANTIC_WALK_INTERNAL_H
#define AMA_SEMANTIC_WALK_INTERNAL_H
#include "semantic/internal.h"
/** Semantic-owned visit record borrowing one uniquely owned AST node and its lexical loop depth. Component contract: [NAME-04](../../../spec/language/declarations.md#name-04), [FLOW-04](../../../spec/language/control-flow.md#flow-04). */
typedef struct SemanticWalkItem {
    struct SemanticWalkItem *next;
    const AstNode *node;
    size_t loop_depth;
    const AstNode *loop;
} SemanticWalkItem;
/** Finite visit sequence with append ownership and checked cardinality. Component contract: [NAME-04](../../../spec/language/declarations.md#name-04), [FLOW-04](../../../spec/language/control-flow.md#flow-04). */
typedef struct { SemanticWalkItem *first; SemanticWalkItem *last; size_t count; } SemanticWalk;
/** Collect every owned structural subtree once, excluding parent/terminal/binding alias links. Component contract: [NAME-04](../../../spec/language/declarations.md#name-04), [FLOW-04](../../../spec/language/control-flow.md#flow-04). */
bool semantic_walk_collect(SemanticProgram *, AstBlock, SemanticWalk *);
/** Stably order visits by original byte position; same-site loop owners precede their command nodes. Component contract: [NAME-04](../../../spec/language/declarations.md#name-04), [FLOW-04](../../../spec/language/control-flow.md#flow-04). */
void semantic_walk_sort(SemanticWalk *);
#endif
