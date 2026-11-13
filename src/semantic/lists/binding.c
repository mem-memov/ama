/** @file
 * @brief Specialize intrinsic input/result field descriptors and apply supplied-source owning-copy requirements.
 * [LIST-04](../../../spec/language/lists.md#list-04) fixes arity/types;
 * [LIST-09](../../../spec/language/lists.md#list-09) never infers clone capability from a possibly empty runtime handle.
 */
#include "internal.h"
bool semantic_list_binding(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding) {
    bool result = argument->kind != AST_INPUT;
    size_t ordinal = argument->position;
    const SemanticListField *field = argument->label == NULL
        ? semantic_list_position(statement->list_method, result, ordinal)
        : semantic_list_name(statement->list_method, result, argument->label->name, &ordinal);
    if (field == NULL) { return semantic_fail(program, argument->span, "No intrinsic field matches this binding", NULL); }
    if (argument->label != NULL && (argument->label->reference != field->element_reference
            || argument->label->whole_list || argument->label->index != NULL)) {
        return semantic_fail(program, argument->label->span, "Binding label storage form differs from its intrinsic field", NULL);
    }
    binding->intrinsic = field;
    binding->type = field->element_reference ? statement->list_interface.element : statement->list_interface.integer;
    binding->position = ordinal;
    binding->reference = field->element_reference;
    return true;
}
bool semantic_list_copy_requirement(SemanticProgram *program, const SemanticStatement *statement,
    const SemanticBinding *binding) {
    if (statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION && !statement->list_interface.reference
        && (statement->list_method == SEMANTIC_LIST_APPEND || statement->list_method == SEMANTIC_LIST_INSERT)
        && binding->syntax->kind == AST_INPUT && !binding->syntax->omitted && binding->intrinsic->element_reference
        && !statement->list_interface.element->cloneable) {
        return semantic_capability_error(program, statement->list_interface.element, ABI_CLONE,
            binding->syntax->span, NULL, "An explicit owning-list source requires independently copyable elements");
    }
    return true;
}
