/**
 * @file
 * @brief Plan one complete ownership copy map with explicit saved-position mode.
 *
 * [COPY-05](../../../spec/language/values-and-references.md#copy-05)
 * Clone owned fields, copy reference handles, fix all internal targets, then preserve or clear positions explicitly.
 */
#include "internal.h"

bool plan_clone(ProgramPlan *plan, PlannedType *type, LifeProcedure *procedure, bool preserve) {
    if (!plan_life_failure(plan, type, procedure, true)
        || !plan_life_append(plan, type, procedure, LIFE_PREFLIGHT_SOURCE, NULL, NULL)
        || !plan_life_append(plan, type, procedure, LIFE_BEGIN_CLONE_MAP, NULL, NULL)) {
        return false;
    }
    for (StorageField *field = type->first; field != NULL; field = field->next) {
        bool reference = field->form == STORAGE_REFERENCE || field->form == STORAGE_REFERENCE_LIST;
        if (!plan_life_append(plan, type, procedure,
                reference ? LIFE_COPY_REFERENCE : LIFE_CLONE_OWNED, field, NULL)) {
            return false;
        }
    }
    if (type->type->module->syntax.sections.kind == MODULE_NATIVE
        && !plan_life_append(plan, type, procedure, LIFE_CLONE_NATIVE, NULL,
            &type->type->abi.bindings[ABI_CLONE])) {
        return false;
    }
    return plan_life_append(plan, type, procedure, LIFE_REMAP_REFERENCES, NULL, NULL)
        && plan_life_append(plan, type, procedure, preserve ? LIFE_PRESERVE_POSITIONS : LIFE_CLEAR_POSITIONS, NULL, NULL)
        && plan_life_append(plan, type, procedure, LIFE_RELEASE_CLONE_MAP, NULL, NULL)
        && plan_life_append(plan, type, procedure, LIFE_MARK_LIVE, NULL, NULL)
        && plan_life_append(plan, type, procedure, LIFE_SUCCESS, NULL, NULL);
}

bool plan_lifecycles(ProgramPlan *plan, PlannedType *type) {
    if (!plan_create(plan, type, &type->create, false) || !plan_destroy(plan, type)) {
        return false;
    }
    if (type->type->abi.bindings[ABI_LITERAL].present
        && !plan_create(plan, type, &type->literal, true)) {
        return false;
    }
    return !type->type->cloneable || (plan_clone(plan, type, &type->clone, false)
        && plan_clone(plan, type, &type->continuation, true));
}
