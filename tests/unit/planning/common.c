/**
 * @file
 * @brief Compose resolved semantics into plans and inspect exact control operation nodes.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_PLANNING_TEST_COMMON
#define AMA_PLANNING_TEST_COMMON

#include "../semantic/typed-common.c"
#include "planning/plan.h"

static ProgramPlan plan;

/**
 * Resolve the fixture, verify relations/constants and build the actual typed plan. Plans borrow
 * semantics until explicit reverse destruction.
 */
static PlanStatus plan_text(const char *text) {
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    status = semantic_relations(&semantic);
    assert(status == SEMANTIC_OK);
    status = semantic_constants(&semantic);
    assert(status == SEMANTIC_OK);
    program_plan_init(&plan, &semantic, sources.allocator);
    return program_plan_build(&plan);
}

/**
 * Destroy plan before semantics/project/source fixture owners, releasing every counted
 * allocation.
 */
static void destroy_plan(void) {
    program_plan_destroy(&plan);
    destroy_semantic();
}

/**
 * Find the first exact requested control operation in the allocated plan list, or null; callers
 * assert its required presence and relationships.
 */
static ControlNode *find_control(const PlannedType *type, ControlOperation operation) {
    for (ControlNode *node = type->controls; node != NULL; node = node->allocated_next) {
        if (node->operation == operation) {
            return node;
        }
    }
    return NULL;
}

#endif
