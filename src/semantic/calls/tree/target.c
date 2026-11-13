/** @file
 * @brief Resolve root targets in the caller and selected targets only in the parent's declared result interface.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) requires
 * exact suffixes and parent identity, without copying or private-name fallback.
 */
#include "../internal.h"
#include "semantic/lists/internal.h"
/** A selected result keeps its declaration's identity; nested indices resolve in the initiating caller. */
static bool selected(SemanticProgram *program, SemanticType *caller, SemanticStatement *statement) {
    const AstNode *node = statement->syntax;
    SemanticStatement *parent = semantic_call_statement(caller, node->as.call.enclosing_operation);
    if (parent == NULL || parent->namespace_type == NULL || parent->operation_kind != SEMANTIC_INSTANCE_OPERATION) {
        return semantic_fail(program, node->span, "The enclosing operation has no ordinary selectable result interface", NULL);
    }
    const AstAccess *target = node->as.call.target;
    SemanticSymbol *field = semantic_interface_name(parent->namespace_type, SYMBOL_RESULT, target->name);
    if (field == NULL) {
        return semantic_fail(program, target->name, "The operation has no such declared result",
            semantic_symbol(parent->namespace_type, target->name));
    }
    statement->enclosing_operation = parent;
    statement->access = semantic_selected_access(program, caller, target, field);
    return statement->access != NULL;
}
bool semantic_call_target(SemanticProgram *program, SemanticType *caller, SemanticStatement *statement) {
    const AstNode *node = statement->syntax;
    if (node->as.call.mode == AST_CONSTANT_READ) {
        statement->operation_kind = SEMANTIC_CONSTANT_OPERATION;
        statement->namespace_type = semantic_type(program, node->as.call.namespace_path);
        return statement->namespace_type != NULL || semantic_fail(program, node->span,
            "The constant namespace is unavailable", NULL);
    }
    if (node->as.call.enclosing_operation != NULL) {
        SemanticStatement *parent = semantic_call_statement(caller, node->as.call.enclosing_operation);
        if (parent != NULL && (parent->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION
                || parent->operation_kind == SEMANTIC_LIST_METHOD_OPERATION)) {
            return semantic_list_target(program, statement, parent);
        }
        if (!selected(program, caller, statement)) { return false; }
    } else {
        statement->access = semantic_access(program, caller, node->as.call.target, false, NULL);
        if (statement->access == NULL) { return false; }
        if (statement->access->symbol->role == SYMBOL_CONSTANT) {
            return semantic_fail(program, node->as.call.target->span,
                "Shared constants cannot be executable or retained-result targets", statement->access->symbol);
        }
    }
    statement->namespace_type = statement->access->symbol->type;
    statement->target_reference = statement->access->reference;
    return !statement->access->list || semantic_list_service(program, statement);
}
