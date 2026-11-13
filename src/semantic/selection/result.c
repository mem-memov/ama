/** @file
 * @brief Resolve one result value for a direct target or terminal condition while keeping all index operands caller-local.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) and
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) share exact interface
 * selection; intrinsic outputs retain ordinal identity without a fake source field.
 */
#include "semantic/internal.h"
bool semantic_select_result(SemanticProgram *program, SemanticType *caller,
    SemanticStatement *parent, const AstAccess *target, SemanticSelection *value) {
    *value = (SemanticSelection){0};
    if (parent == NULL) { return semantic_fail(program, target->span, "The parent result operation is unavailable", NULL); }
    if (parent->operation_kind == SEMANTIC_LIST_METHOD_OPERATION) {
        const SemanticListField *field = semantic_list_name(parent->list_method, true, target->name, &value->position);
        if (field == NULL) { return semantic_fail(program, target->span, "No intrinsic result matches this selector", NULL); }
        if (target->reference != field->element_reference || target->whole_list || target->index != NULL) {
            return semantic_fail(program, target->span, "Selected storage form differs from its intrinsic result", NULL);
        }
        value->type = field->element_reference ? parent->list_interface.element : parent->list_interface.integer;
        value->reference = field->element_reference;
        value->intrinsic = true;
        return true;
    }
    if (parent->operation_kind != SEMANTIC_INSTANCE_OPERATION || parent->namespace_type == NULL) {
        return semantic_fail(program, target->span, "The enclosing operation has no ordinary selectable result interface", NULL);
    }
    value->field = semantic_interface_name(parent->namespace_type, SYMBOL_RESULT, target->name);
    if (value->field == NULL) {
        return semantic_fail(program, target->name, "The operation has no such declared result", semantic_symbol(parent->namespace_type, target->name));
    }
    value->access = semantic_selected_access(program, caller, target, value->field);
    if (value->access == NULL) { return false; }
    value->type = value->field->type;
    value->reference = value->access->reference;
    value->list = value->access->list;
    return true;
}
