/**
 * @file
 * @brief Plan parameters/results in callee declaration order with exactly selected input bindings.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Later defaults must observe earlier installed parameters; persistent state fields are excluded from reset.
 */
#include "internal.h"

/** Find one nonomitted resolved input by exact interface ordinal; duplicates were rejected semantically and omitted inputs select defaults.
 */
static const SemanticBinding *input(const SemanticStatement *statement, size_t position) {
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind == AST_INPUT && binding->position == position && !binding->syntax->omitted) {
            return binding;
        }
    }
    return NULL;
}

ControlNode *lower_install(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after) {
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
