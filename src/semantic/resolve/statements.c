/**
 * @file
 * @brief Resolve retained assignments, calls and condition/control operands.
 *
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02)
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) and
 * [FLOW-05](../../../spec/language/lists.md#flow-05) retain explicit incoming
 * loop identities; terminal conditions resolve only after complete header inventories.
 */
#include "semantic/internal.h"

/** Resolve one retained statement and its exact AST identity; reject break outside loops before planning and preserve caller-side operand context.
 * Component contract: [COMP-02](../../../spec/implementation/compiler.md#comp-02), [FLOW-04](../../../spec/language/control-flow.md#flow-04), [FLOW-05](../../../spec/language/lists.md#flow-05). */
static bool statement(SemanticProgram *program, SemanticType *type, const AstNode *node, size_t loop_depth, const AstNode *loop, void *context) {
    (void)context;
    if (node->kind == AST_BREAK && loop_depth == 0) {
        return semantic_fail(program, node->span, "Loop exit ^ requires an enclosing loop", NULL);
    }
    SemanticStatement *resolved = semantic_allocate(program, sizeof(*resolved));
    if (resolved == NULL) {
        return false;
    }
    resolved->syntax = node;
    resolved->enclosing_loop = loop == NULL ? NULL : semantic_statement_find(type, loop);
    if (type->last_statement == NULL) {
        type->statements = resolved;
    } else {
        type->last_statement->next = resolved;
    }
    type->last_statement = resolved;
    if (node->kind == AST_ASSIGNMENT) {
        resolved->access = semantic_access(program, type, node->as.assignment.receiver, true, NULL);
        if (resolved->access == NULL) {
            return false;
        }
        resolved->source = semantic_source(program, type, node->as.assignment.source, NULL);
        return resolved->access != NULL && resolved->source != NULL
            && semantic_transfer(program, resolved->access->symbol->type, resolved->access->reference,
                resolved->access->list, resolved->source, resolved->access,
                resolved->access->symbol, &resolved->transfer);
    }
    if (node->kind == AST_CALL) {
        return semantic_call(program, type, resolved);
    }
    return true;
}

bool semantic_statements(SemanticProgram *program, SemanticType *type) {
    return semantic_walk(program, type, type->module->syntax.body, statement, NULL)
        && semantic_output_actions(program, type) && semantic_controls(program, type);
}
