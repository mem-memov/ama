/**
 * @file
 * @brief Compose complete structural collection with source-ordered semantic callbacks.
 * [NAME-04](../../../spec/language/declarations.md#name-04) requires one complete
 * explicit inventory before later-use checks; [ARCH-05](../../../spec/implementation/compiler.md#arch-05)
 * uses exact byte ordering and explicit loop-depth metadata, without recursive C frames.
 */
#include "internal.h"
bool semantic_walk(SemanticProgram *program, SemanticType *type, AstBlock block,
    SemanticVisitor visit, void *context) {
    SemanticWalk walk = {0};
    if (!semantic_walk_collect(program, block, &walk)) { return false; }
    semantic_walk_sort(&walk);
    for (const SemanticWalkItem *item = walk.first; item != NULL; item = item->next) {
        if (!visit(program, type, item->node, item->loop_depth, context)) { return false; }
    }
    return true;
}
