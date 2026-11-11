#include "internal.h"

static SemanticSymbol *positional(SemanticType *type, SymbolRole role, size_t position) {
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role == role && symbol->role_ordinal == position) {
            return symbol;
        }
    }
    return NULL;
}

bool semantic_call_field(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding) {
    SemanticType *type = statement->namespace_type;
    SymbolRole role = statement->syntax->as.call.mode == AST_CONSTANT_READ ? SYMBOL_CONSTANT
        : argument->kind == AST_INPUT ? SYMBOL_PARAMETER : SYMBOL_RESULT;
    SemanticSymbol *field = argument->label == NULL ? positional(type, role, argument->position)
        : semantic_symbol(type, argument->label->name);
    if (field == NULL || field->role != role) {
        return semantic_fail(program, argument->span, "No interface field matches this binding", field);
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
