/** @file
 * @brief Resolve fixed intrinsic method selectors and further calls on their ephemeral typed results.
 * [LIST-04](../../../spec/language/lists.md#list-04) forbids method values and
 * resume/read modes while [CALL-09](../../../spec/language/calls-and-results.md#call-09)
 * permits ordinary operations on typed element/length results without fake fields.
 */
#include "internal.h"
bool semantic_list_target(SemanticProgram *program, SemanticStatement *statement, SemanticStatement *parent) {
    const AstNode *node = statement->syntax;
    const AstAccess *target = node->as.call.target;
    statement->enclosing_operation = parent;
    statement->list_interface = parent->list_interface;
    statement->list_service = parent->list_service;
    if (parent->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION) {
        if (!semantic_list_method(target->name, &statement->list_method)) {
            return semantic_fail(program, target->name, "The list service has no such method", NULL);
        }
        if (target->reference || target->whole_list || target->index != NULL || node->as.call.mode != AST_ENTRY_CALL) {
            return semantic_fail(program, target->span, "A list method requires its plain declared name and entry mode", NULL);
        }
        statement->operation_kind = SEMANTIC_LIST_METHOD_OPERATION;
        return true;
    }
    size_t ordinal = 0;
    const SemanticListField *field = semantic_list_name(parent->list_method, true, target->name, &ordinal);
    if (field == NULL) { return semantic_fail(program, target->span, "No intrinsic result matches this selector", NULL); }
    if (target->reference != field->element_reference || target->whole_list || target->index != NULL) {
        return semantic_fail(program, target->span, "Selected storage form differs from its intrinsic result", NULL);
    }
    statement->intrinsic_target = true;
    statement->intrinsic_result = ordinal;
    statement->target_reference = field->element_reference;
    statement->namespace_type = field->element_reference ? parent->list_interface.element : parent->list_interface.integer;
    return true;
}
