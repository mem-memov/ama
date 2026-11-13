/**
 * @file
 * @brief Resolve named/positional interface fields and reject duplicate bindings.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Direction role and exact storage form must match the callee declaration; placeholders still consume positions.
 */
#include "internal.h"

bool semantic_call_field(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding) {
    SemanticType *type = statement->namespace_type;
    SymbolRole role = statement->syntax->as.call.mode == AST_CONSTANT_READ ? SYMBOL_CONSTANT
        : argument->kind == AST_INPUT ? SYMBOL_PARAMETER : SYMBOL_RESULT;
    SemanticSymbol *field = argument->label == NULL ? semantic_interface_position(type, role, argument->position)
        : semantic_interface_name(type, role, argument->label->name);
    if (field == NULL) {
        SemanticSymbol *related = argument->label == NULL ? NULL : semantic_symbol(type, argument->label->name);
        return semantic_fail(program, argument->span, "No interface field matches this binding", related);
    }
    if (argument->label != NULL && (argument->label->reference != field->reference
            || argument->label->whole_list != field->list)) {
        return semantic_fail(program, argument->label->span, "Binding label storage form differs from its field", field);
    }
    for (SemanticBinding *earlier = statement->bindings; earlier != NULL; earlier = earlier->next) {
        if (earlier->field == field && earlier->syntax->kind == argument->kind) {
            semantic_fail(program, argument->span, "The interface field is bound more than once", field);
            program->related[0].location.span = earlier->syntax->span;
            program->related[0].role = "earlier binding";
            return false;
        }
    }
    binding->field = field;
    binding->type = field->type;
    binding->position = field->role_ordinal;
    binding->reference = field->reference;
    binding->list = field->list;
    return true;
}
