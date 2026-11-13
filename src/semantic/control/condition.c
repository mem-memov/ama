/** @file
 * @brief Resolve terminal truth values after all header operations exist and retain the exact root restarted by each iteration.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) restarts the full
 * linear command, while [FLOW-01](../../../spec/language/control-flow.md#flow-01)
 * requires each terminal predicate's own native condition capability.
 */
#include "internal.h"
/** Resolve a caller-local predicate or one exact parent-result selection, without evaluating any runtime condition. */
bool semantic_control_condition(SemanticProgram *program, SemanticType *type, SemanticStatement *statement) {
    const AstNode *node = statement->syntax;
    SemanticSelection *value = &statement->condition;
    if (node->as.control.condition_owner != NULL) {
        statement->condition_owner = semantic_statement_find(type, node->as.control.condition_owner);
        if (!semantic_select_result(program, type, statement->condition_owner, node->as.control.condition, value)) { return false; }
        statement->loop_root = semantic_statement_find(type, node->as.control.command);
        if (statement->loop_root == NULL) { return semantic_fail(program, node->span, "The linear loop header has no resolved restart root", NULL); }
    } else {
        value->access = semantic_access(program, type, node->as.control.condition, false, NULL);
        if (value->access == NULL) { return false; }
        value->field = value->access->symbol;
        value->type = value->field->type;
        value->reference = value->access->reference;
        value->list = value->access->list;
        if (node->kind == AST_LOOP) { statement->loop_root = statement; }
    }
    statement->access = value->access;
    return semantic_conversion(program, value->type, value->reference, value->list,
        ABI_CONDITION, node->as.control.condition->span, value->field);
}
