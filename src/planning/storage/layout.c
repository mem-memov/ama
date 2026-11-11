#include "planning/internal.h"

PlannedType *program_plan_type(const ProgramPlan *plan, const SemanticType *type) {
    for (PlannedType *candidate = plan->first; candidate != NULL; candidate = candidate->next) {
        if (candidate->type == type) {
            return candidate;
        }
    }
    return NULL;
}

static bool fields(ProgramPlan *plan, PlannedType *type) {
    for (SemanticSymbol *symbol = type->type->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role == SYMBOL_CONSTANT) {
            continue;
        }
        StorageField *field = plan_allocate(plan, sizeof(*field));
        if (field == NULL) {
            return false;
        }
        *field = (StorageField){.symbol = symbol, .previous = type->last,
            .type = program_plan_type(plan, symbol->type), .slot = type->field_count++,
            .form = symbol->list ? (symbol->reference ? STORAGE_REFERENCE_LIST : STORAGE_OWNING_LIST)
                : symbol->reference ? STORAGE_REFERENCE : STORAGE_OWNED};
        if (type->last == NULL) {
            type->first = field;
        } else {
            type->last->next = field;
        }
        type->last = field;
    }
    return true;
}

static bool shared_storage(ProgramPlan *plan) {
    for (const SemanticConstantPlan *constant = plan->semantic->constant_first;
        constant != NULL; constant = constant->next) {
        SharedStorage *storage = plan_allocate(plan, sizeof(*storage));
        if (storage == NULL) {
            return false;
        }
        *storage = (SharedStorage){.previous = plan->shared_last, .constant = constant,
            .value_type = program_plan_type(plan, constant->symbol->type), .immutable = true,
            .form = constant->symbol->list ? STORAGE_OWNING_LIST : STORAGE_OWNED};
        if (plan->shared_last == NULL) {
            plan->shared_first = storage;
        } else {
            plan->shared_last->next = storage;
        }
        plan->shared_last = storage;
    }
    return true;
}

bool plan_storage(ProgramPlan *plan) {
    for (SemanticType *semantic = plan->semantic->first; semantic != NULL; semantic = semantic->next) {
        PlannedType *type = plan_allocate(plan, sizeof(*type));
        if (type == NULL) {
            return false;
        }
        type->type = semantic;
        type->requires_pool = semantic->executable;
        type->service_storage = semantic->executable ? STORAGE_GENERATION | STORAGE_BUSY
            | STORAGE_POSITION | STORAGE_RETURN_LINK | STORAGE_REFUSAL | STORAGE_ACQUISITION : 0;
        if (plan->last == NULL) {
            plan->first = type;
        } else {
            plan->last->next = type;
        }
        plan->last = type;
        if (semantic->module == plan->semantic->project->entry) {
            plan->entry = type;
        }
    }
    for (PlannedType *type = plan->first; type != NULL; type = type->next) {
        if (!fields(plan, type)) {
            return false;
        }
    }
    size_t ordinal = 0;
    for (GraphVertex *vertex = plan->semantic->eager.order; vertex != NULL; vertex = vertex->order_next) {
        program_plan_type(plan, vertex->value)->layout_ordinal = ordinal++;
    }
    return shared_storage(plan);
}
