#include <stdint.h>
#include "internal.h"

ControlNode *plan_control_node(ProgramPlan *plan, PlannedType *type,
    ControlOperation operation, const SemanticStatement *statement) {
    if (plan->next_selector == SIZE_MAX) {
        plan->status = PLAN_OUT_OF_MEMORY;
        plan->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "The control selector space is exhausted"};
        return NULL;
    }
    ControlNode *node = plan_allocate(plan, sizeof(*node));
    if (node != NULL) {
        *node = (ControlNode){.allocated_next = type->controls, .operation = operation,
            .selector = plan->next_selector++, .type = type, .statement = statement,
            .failure = type->failure, .site = statement == NULL
                ? (SourceSpan){type->type->module->source->text, 0, 0} : statement->syntax->span};
        type->controls = node;
    }
    return node;
}

ControlNode *lower_prepend(Lowering *lowering, ControlOperation operation,
    const SemanticStatement *statement, const SemanticBinding *binding, ControlNode *after) {
    ControlNode *node = plan_control_node(lowering->plan, lowering->type, operation, statement);
    if (node != NULL) {
        node->binding = binding;
        node->next = after;
        if (binding != NULL) {
            node->site = binding->syntax->span;
            node->position = binding->position;
            node->field = binding->field;
            node->value_type = binding->type;
            node->value_form = binding->list ? (binding->reference ? STORAGE_REFERENCE_LIST
                : STORAGE_OWNING_LIST) : binding->reference ? STORAGE_REFERENCE : STORAGE_OWNED;
        }
    }
    return node;
}

const SemanticStatement *plan_statement(const PlannedType *type, const AstNode *syntax) {
    for (const SemanticStatement *statement = type->type->statements;
        statement != NULL; statement = statement->next) {
        if (statement->syntax == syntax) {
            return statement;
        }
    }
    return NULL;
}
