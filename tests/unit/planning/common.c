#ifndef AMA_PLANNING_TEST_COMMON
#define AMA_PLANNING_TEST_COMMON

#include "../semantic/typed-common.c"
#include "planning/plan.h"

static ProgramPlan plan;

static PlanStatus plan_libraries(const char *text, SemanticLibraries libraries) {
    SemanticStatus status = resolve_text(text, libraries);
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    status = semantic_constants(&semantic);
    assert(status == SEMANTIC_OK);
    program_plan_init(&plan, &semantic, sources.allocator);
    return program_plan_build(&plan);
}

static PlanStatus plan_text(const char *text) {
    return plan_libraries(text, (SemanticLibraries){0});
}

static void destroy_plan(void) {
    program_plan_destroy(&plan);
    destroy_semantic();
}

static ControlNode *find_control(const PlannedType *type, ControlOperation operation) {
    for (ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node->operation == operation) {
            return node;
        }
    }
    return NULL;
}

#endif
