#include <assert.h>
#include <string.h>
#include "internal.h"

void program_plan_init(ProgramPlan *plan, const SemanticProgram *semantic, SourceAllocator allocator) {
    assert(semantic->constants_valid);
    *plan = (ProgramPlan){.semantic = semantic, .next_selector = 1};
    source_arena_init(&plan->arena, allocator);
}

void program_plan_destroy(ProgramPlan *plan) {
    source_arena_destroy(&plan->arena);
    *plan = (ProgramPlan){0};
}

void *plan_allocate(ProgramPlan *plan, size_t size) {
    void *allocation = source_arena_allocate(&plan->arena, size);
    if (allocation == NULL) {
        plan->status = PLAN_OUT_OF_MEMORY;
        plan->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Typed planning could not allocate storage"};
    } else {
        memset(allocation, 0, size);
    }
    return allocation;
}

bool plan_fail(ProgramPlan *plan, SourceSpan site, const char *message) {
    plan->status = PLAN_INVALID;
    plan->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
        .message = message, .primary = {.span = site}};
    return false;
}

PlanStatus program_plan_build(ProgramPlan *plan) {
    assert(!plan->attempted);
    plan->attempted = true;
    if (!plan_storage(plan)) {
        return plan->status;
    }
    for (PlannedType *type = plan->first; type != NULL; type = type->next) {
        if (type->requires_pool && (!plan_lifecycles(plan, type) || !plan_control(plan, type))) {
            return plan->status;
        }
    }
    /* All entries now exist, including forward/cyclic module call targets. */
    for (PlannedType *type = plan->first; type != NULL; type = type->next) {
        for (ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
            if (node->operation == CONTROL_ENTER_INSTANCE) {
                node->next = node->callee->entry;
            }
        }
    }
    plan->constant_startup = plan->semantic->constant_first;
    plan->constant_cleanup = plan->semantic->constant_last;
    plan->valid = true;
    return PLAN_OK;
}
