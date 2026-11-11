#include "internal.h"

bool semantic_list_field(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding) {
    if (argument->label != NULL) {
        return semantic_fail(program, argument->label->span, "Whole-list bindings must be positional", NULL);
    }
    bool input = argument->kind == AST_INPUT;
    size_t position = argument->position;
    if (position >= (input ? 4u : 3u)) {
        return semantic_fail(program, argument->marker, "The whole-list binding exceeds its fixed interface", NULL);
    }
    size_t role = input ? (position == 0 ? 0 : 1) : position == 0 ? 1 : position == 1 ? 2 : 3;
    binding->type = input && position == 2 ? statement->access->symbol->type : program->library_types[role];
    binding->position = position;
    binding->reference = input && position == 2;
    if (binding->type == NULL || !binding->type->executable) {
        return semantic_fail(program, argument->span, "A required list-library type mapping is unavailable", NULL);
    }
    if (input && position == 2 && !argument->omitted && !statement->access->reference
        && !statement->access->symbol->type->cloneable) {
        return semantic_fail(program, argument->span,
            "Supplying an owning-list source requires element clone capability", statement->access->symbol);
    }
    return true;
}
