/**
 * @file
 * @brief Compose semantic analysis and planning with explicit partial-owner lifetime.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Set initialization credits before fallible work; destroy plans before semantics and loaded sources.
 */
#include <assert.h>
#include "internal.h"

/**
 * Initialize source/project owners and empty semantic/plan credits with the supplied allocator;
 * check once before destruction.
 */
void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator) {
    *frontend = (CompilerFrontend){.allocator = allocator};
    project_init(&frontend->project, allocator);
    source_manager_init(&frontend->proposed_sources, allocator);
}

/**
 * Destroy initialized plan before semantic state, then proposed ABI sources and project storage;
 * reset the owner so borrowed outputs expire together.
 */
void compiler_frontend_destroy(CompilerFrontend *frontend) {
    if (frontend->plan_initialized) {
        program_plan_destroy(&frontend->plan);
    }
    if (frontend->semantic_initialized) {
        semantic_program_destroy(&frontend->semantic);
    }
    source_manager_destroy(&frontend->proposed_sources);
    project_destroy(&frontend->project);
    *frontend = (CompilerFrontend){0};
}

/**
 * Analyze the loaded closure using an optional explicit authoring contract, then build resolved
 * plans. Record initialization before fallible work and retain the first stage diagnostic.
 */
bool compiler_frontend_finish(CompilerFrontend *frontend,
    const SemanticNativeAuthoring *authoring) {
    semantic_program_init(&frontend->semantic, &frontend->project, frontend->allocator);
    frontend->semantic_initialized = true;
    if (semantic_analyze_authoring(&frontend->semantic, authoring) != SEMANTIC_OK) {
        frontend->error = frontend->semantic.error;
        return false;
    }
    program_plan_init(&frontend->plan, &frontend->semantic, frontend->allocator);
    frontend->plan_initialized = true;
    if (program_plan_build(&frontend->plan) != PLAN_OK) {
        frontend->error = frontend->plan.error;
        return false;
    }
    frontend->valid = true;
    return true;
}

/**
 * Load exactly once, then perform strict semantic/planning completion without authoring
 * overrides; false leaves safely destructible diagnostic state.
 */
bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry) {
    return compiler_frontend_load(frontend, roots, count, entry)
        && compiler_frontend_finish(frontend, NULL);
}
