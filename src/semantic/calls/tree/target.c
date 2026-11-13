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
    SemanticStatement *parent = semantic_statement_find(caller, node->as.call.enclosing_operation);
    SemanticSelection value;
    if (!semantic_select_result(program, caller, parent, node->as.call.target, &value)) { return false; }
    statement->enclosing_operation = parent;
    statement->access = value.access;
    statement->namespace_type = value.type;
    statement->target_reference = value.reference;
    statement->intrinsic_target = value.intrinsic;
    statement->intrinsic_result = value.position;
    return !value.list || semantic_list_service(program, statement);
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
        SemanticStatement *parent = semantic_statement_find(caller, node->as.call.enclosing_operation);
        if (parent != NULL && parent->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION) {
            return semantic_list_target(program, statement, parent);
        }
        return selected(program, caller, statement);
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
