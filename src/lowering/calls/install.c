#include "internal.h"

static const SemanticBinding *input(const SemanticStatement *statement, size_t position) {
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind == AST_INPUT && binding->position == position && !binding->syntax->omitted) {
            return binding;
        }
    }
    return NULL;
}

ControlNode *lower_install(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, bool list) {
    if (list) {
        for (size_t position = 4; position > 0; position--) {
            after = lower_prepend(lowering, CONTROL_LIST_INSTALL_INPUT, statement, input(statement, position - 1), after);
            if (after == NULL) {
                return NULL;
            }
            after->position = position - 1;
            after->value_type = position == 3 ? statement->access->symbol->type
                : lowering->plan->semantic->library_types[position == 1 ? 0 : 1];
            after->value_form = position == 3 ? STORAGE_REFERENCE : STORAGE_OWNED;
        }
        return after;
    }
    PlannedType *callee = program_plan_type(lowering->plan, statement->namespace_type);
    for (StorageField *field = callee->last; field != NULL; field = field->previous) {
        SymbolRole role = field->symbol->role;
        if (role == SYMBOL_PARAMETER || role == SYMBOL_RESULT) {
            after = lower_prepend(lowering, role == SYMBOL_PARAMETER ? CONTROL_INSTALL_PARAMETER
                : CONTROL_INITIALIZE_RESULT, statement, role == SYMBOL_PARAMETER
                    ? input(statement, field->symbol->role_ordinal) : NULL, after);
            if (after == NULL) {
                return NULL;
            }
            after->field = field->symbol;
            after->value_type = field->symbol->type;
            after->value_form = field->form;
            after->position = field->symbol->role_ordinal;
        }
    }
    return after;
}
