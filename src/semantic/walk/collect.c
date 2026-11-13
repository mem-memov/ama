/**
 * @file
 * @brief Collect all structural statement views while retaining exact lexical loop context.
 * [NAME-04](../../../spec/language/declarations.md#name-04) includes declarations
 * inside selected handlers and traversal bodies. [FLOW-04](../../../spec/language/control-flow.md#flow-04)
 * increments loop depth only for bodies, never a folded command's handlers.
 */
#include <stdint.h>
#include "internal.h"
/** Semantic-owned cursor for one borrowed AST subtree and its incoming loop depth. */
typedef struct WalkFrame {
    struct WalkFrame *next;
    const AstNode *node;
    size_t loop_depth;
    const AstNode *loop;
} WalkFrame;
/** Queue one nonempty owning subtree; null nodes require no allocation. */
static bool push(SemanticProgram *program, WalkFrame **pending,
    const AstNode *node, size_t loop_depth, const AstNode *loop) {
    if (node == NULL) { return true; }
    WalkFrame *frame = semantic_allocate(program, sizeof(*frame));
    if (frame == NULL) { return false; }
    *frame = (WalkFrame){*pending, node, loop_depth, loop};
    *pending = frame;
    return true;
}
/** Schedule each owned child exactly once; aliases never supply additional traversal edges. */
static bool children(SemanticProgram *program, WalkFrame **pending,
    const AstNode *node, size_t depth, const AstNode *loop) {
    if (node->kind == AST_CALL) {
        if (!push(program, pending, node->as.call.alternative.first, depth, loop)) { return false; }
        for (const AstOutputAction *action = node->as.call.first_output; action != NULL; action = action->next) {
            if (action->kind == AST_SELECTED_ACTION && !push(program, pending, action->operation, depth, loop)) { return false; }
        }
    } else if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        if (node->kind == AST_LOOP && depth == SIZE_MAX) { return semantic_memory_failure(program); }
        if (!push(program, pending, node->as.control.alternative.first, depth, loop)
            || !push(program, pending, node->as.control.body.first, depth + (node->kind == AST_LOOP), node->kind == AST_LOOP ? node : loop)
            || !push(program, pending, node->as.control.command, depth, loop)) { return false; }
    } else if (node->kind == AST_FOREACH) {
        if (depth == SIZE_MAX) { return semantic_memory_failure(program); }
        return push(program, pending, node->as.traversal.body.first, depth + 1, node);
    }
    return true;
}
/** Append one stable event before collecting its children; equal-site loop/command order remains explicit. */
static bool append(SemanticProgram *program, SemanticWalk *walk, const AstNode *node, size_t depth, const AstNode *loop) {
    if (walk->count == SIZE_MAX) { return semantic_memory_failure(program); }
    SemanticWalkItem *item = semantic_allocate(program, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (SemanticWalkItem){.node = node, .loop_depth = depth, .loop = loop};
    if (walk->last == NULL) { walk->first = item; }
    else { walk->last->next = item; }
    walk->last = item;
    walk->count++;
    return true;
}
bool semantic_walk_collect(SemanticProgram *program, AstBlock block, SemanticWalk *walk) {
    WalkFrame *pending = NULL;
    if (!push(program, &pending, block.first, 0, NULL)) { return false; }
    while (pending != NULL) {
        const AstNode *node = pending->node;
        if (node == NULL) { pending = pending->next; continue; }
        pending->node = node->next;
        size_t depth = pending->loop_depth;
        const AstNode *loop = pending->loop;
        if (!append(program, walk, node, depth, loop) || !children(program, &pending, node, depth, loop)) { return false; }
    }
    return true;
}
