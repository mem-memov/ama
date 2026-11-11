#include "internal.h"

bool plan_create(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool literal) {
    if (!plan_life_failure(plan, type, procedure, false)) {
        return false;
    }
    for (StorageField *field = type->first; field != NULL; field = field->next) {
        if (!plan_life_append(plan, type, procedure, LIFE_CONSTRUCT_FIELD, field, NULL)) {
            return false;
        }
    }
    if (type->type->module->syntax.sections.kind == MODULE_NATIVE
        && !plan_life_append(plan, type, procedure, LIFE_CONSTRUCT_NATIVE, NULL,
            &type->type->abi.bindings[literal ? ABI_LITERAL : ABI_DEFAULT])) {
        return false;
    }
    return plan_life_append(plan, type, procedure, LIFE_MARK_LIVE, NULL, NULL)
        && plan_life_append(plan, type, procedure, LIFE_SUCCESS, NULL, NULL);
}

bool plan_destroy(ProgramPlan *plan, PlannedType *type) {
    LifeProcedure *procedure = &type->destroy;
    procedure->accumulates_errors = true;
    if (type->type->module->syntax.sections.kind == MODULE_NATIVE
        && !plan_life_append(plan, type, procedure, LIFE_DESTROY_NATIVE, NULL,
            &type->type->abi.bindings[ABI_DESTROY])) {
        return false;
    }
    for (StorageField *field = type->last; field != NULL; field = field->previous) {
        if (!plan_life_append(plan, type, procedure, LIFE_DESTROY_FIELD, field, NULL)) {
            return false;
        }
    }
    return plan_life_append(plan, type, procedure, LIFE_MARK_FINALIZED, NULL, NULL)
        && plan_life_append(plan, type, procedure, LIFE_SUCCESS, NULL, NULL);
}
