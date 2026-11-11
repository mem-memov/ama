#include "internal.h"

static SemanticSource *output_source(SemanticProgram *program, SemanticBinding *binding) {
    SemanticSource *source = semantic_allocate(program, sizeof(*source));
    if (source == NULL) {
        return NULL;
    }
    *source = (SemanticSource){.site = binding->syntax->span, .kind = AST_VALUE_SOURCE,
        .type = binding->type, .declaration = binding->field,
        .reference = binding->reference, .list = binding->list,
        .constant = binding->field != NULL && binding->field->role == SYMBOL_CONSTANT
            ? binding->field : NULL};
    return source;
}

bool semantic_binding_value(SemanticProgram *program, SemanticType *caller,
    SemanticStatement *statement, SemanticBinding *binding) {
    (void)statement;
    const AstArgument *argument = binding->syntax;
    if (argument->omitted) {
        binding->transfer = TRANSFER_DEFAULT;
        return true;
    }
    if (argument->kind == AST_INPUT) {
        binding->source = semantic_source(program, caller, argument->source, NULL);
        return binding->source != NULL && semantic_transfer(program, binding->type,
            binding->reference, binding->list, binding->source, NULL, binding->field, &binding->transfer);
    }
    binding->receiver = semantic_access(program, caller, argument->receiver, true, NULL);
    if (binding->receiver == NULL) {
        return false;
    }
    binding->source = output_source(program, binding);
    return binding->receiver != NULL && binding->source != NULL
        && semantic_transfer(program, binding->receiver->symbol->type, binding->receiver->reference,
            binding->receiver->list, binding->source, binding->receiver,
            binding->receiver->symbol, &binding->transfer);
}
