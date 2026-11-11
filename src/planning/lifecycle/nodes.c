#include "internal.h"

LifeNode *plan_life_node(ProgramPlan *plan, PlannedType *type, LifeOperation operation,
    StorageField *field, const AbiBinding *binding) {
    LifeNode *node = plan_allocate(plan, sizeof(*node));
    if (node != NULL) {
        *node = (LifeNode){.operation = operation, .type = type, .field = field, .binding = binding,
            .site = field == NULL ? (SourceSpan){type->type->module->source->text, 0, 0}
                : field->symbol->declaration->span};
    }
    return node;
}

bool plan_life_append(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure,
    LifeOperation operation, StorageField *field, const AbiBinding *binding) {
    LifeNode *node = plan_life_node(plan, type, operation, field, binding);
    if (node == NULL) {
        return false;
    }
    node->failure = procedure->failure;
    if (procedure->last == NULL) {
        procedure->first = node;
    } else {
        procedure->last->next = node;
    }
    procedure->last = node;
    return true;
}

bool plan_life_failure(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool clone) {
    LifeNode *unwind = plan_life_node(plan, type, LIFE_UNWIND_PARTIAL, NULL, NULL);
    LifeNode *failed = plan_life_node(plan, type, LIFE_FAILURE, NULL, NULL);
    if (unwind == NULL || failed == NULL) {
        return false;
    }
    unwind->next = failed;
    if (clone) {
        LifeNode *release = plan_life_node(plan, type, LIFE_RELEASE_CLONE_MAP, NULL, NULL);
        if (release == NULL) {
            return false;
        }
        unwind->next = release;
        release->next = failed;
    }
    procedure->failure = unwind;
    return true;
}
