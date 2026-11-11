#include "semantic/internal.h"

typedef struct WalkFrame {
    struct WalkFrame *next;
    const AstNode *node;
    size_t loop_depth;
} WalkFrame;

static bool push(SemanticProgram *program, WalkFrame **pending, AstBlock block, size_t loop_depth) {
    if (block.first == NULL) {
        return true;
    }
    WalkFrame *frame = semantic_allocate(program, sizeof(*frame));
    if (frame == NULL) {
        return false;
    }
    *frame = (WalkFrame){*pending, block.first, loop_depth};
    *pending = frame;
    return true;
}

static bool children(SemanticProgram *program, WalkFrame **pending, const AstNode *node, size_t loop_depth) {
    if (node->kind == AST_CALL) {
        return push(program, pending, node->as.call.alternative, loop_depth);
    }
    if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        return push(program, pending, node->as.control.alternative, loop_depth)
            && push(program, pending, node->as.control.body, loop_depth + (node->kind == AST_LOOP));
    }
    return true;
}

bool semantic_walk(SemanticProgram *program, SemanticType *type, AstBlock block,
    SemanticVisitor visit, void *context) {
    WalkFrame *pending = NULL;
    if (!push(program, &pending, block, 0)) {
        return false;
    }
    while (pending != NULL) {
        const AstNode *node = pending->node;
        if (node == NULL) {
            pending = pending->next;
            continue;
        }
        pending->node = node->next;
        size_t loop_depth = pending->loop_depth;
        if (!visit(program, type, node, loop_depth, context)
            || !children(program, &pending, node, loop_depth)) {
            return false;
        }
    }
    return true;
}
