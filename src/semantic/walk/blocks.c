#include "semantic/internal.h"

typedef struct WalkFrame {
    struct WalkFrame *next;
    const AstNode *node;
} WalkFrame;

static bool push(SemanticProgram *program, WalkFrame **pending, AstBlock block) {
    if (block.first == NULL) {
        return true;
    }
    WalkFrame *frame = semantic_allocate(program, sizeof(*frame));
    if (frame == NULL) {
        return false;
    }
    *frame = (WalkFrame){*pending, block.first};
    *pending = frame;
    return true;
}

static bool children(SemanticProgram *program, WalkFrame **pending, const AstNode *node) {
    if (node->kind == AST_CALL) {
        return push(program, pending, node->as.call.alternative);
    }
    if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        return push(program, pending, node->as.control.alternative)
            && push(program, pending, node->as.control.body);
    }
    return true;
}

bool semantic_walk(SemanticProgram *program, SemanticType *type, AstBlock block,
    SemanticVisitor visit, void *context) {
    WalkFrame *pending = NULL;
    if (!push(program, &pending, block)) {
        return false;
    }
    while (pending != NULL) {
        const AstNode *node = pending->node;
        if (node == NULL) {
            pending = pending->next;
            continue;
        }
        pending->node = node->next;
        if (!visit(program, type, node, context) || !children(program, &pending, node)) {
            return false;
        }
    }
    return true;
}
