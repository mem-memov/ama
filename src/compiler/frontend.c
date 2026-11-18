/**
 * @file
 * @brief Compose semantic analysis and planning with explicit partial-owner lifetime.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Set initialization credits before fallible work; destroy plans before semantics and loaded sources.
 * [TPL-13](../../spec/language/templates.md#tpl-13) also gates preloaded catalog
 * checks before input capture and semantic/header preparation.
 */
#include <assert.h>
#include "internal.h"

/**
 * Initialize source/project owners and empty semantic/plan credits with the supplied allocator;
 * check once before destruction.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator) {
    *frontend = (CompilerFrontend){.allocator = allocator};
    project_init(&frontend->project, allocator);
    project_inputs_init(&frontend->inputs, allocator);
    source_manager_init(&frontend->proposed_sources, allocator);
}

/**
 * Destroy initialized plan before semantic state, then proposed ABI sources and project storage;
 * release static metadata before Project/source destruction and reset the owner
 * so borrowed outputs expire together.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
void compiler_frontend_destroy(CompilerFrontend *frontend) {
    if (frontend->plan_initialized) {
        program_plan_destroy(&frontend->plan);
    }
    if (frontend->semantic_initialized) {
        semantic_program_destroy(&frontend->semantic);
    }
    if (frontend->static_initialized) {
        static_inventory_destroy(&frontend->static_inventory);
    }
    source_manager_destroy(&frontend->proposed_sources);
    project_inputs_destroy(&frontend->inputs);
    project_destroy(&frontend->project);
    *frontend = (CompilerFrontend){0};
}

/**
 * Analyze the loaded closure using an optional explicit authoring contract, then build resolved
 * plans. Record initialization before fallible work and retain the first stage diagnostic.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
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
    project_inputs_seal(&frontend->inputs);
    SourceStatus compared = project_inputs_compare(&frontend->inputs, NULL);
    assert(compared == SOURCE_OK);
    (void)compared;
    frontend->valid = true;
    return true;
}

/**
 * Load exactly once, then perform strict semantic/planning completion without authoring
 * overrides; false leaves safely destructible diagnostic state.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry) {
    return compiler_frontend_load(frontend, roots, count, entry)
        && compiler_frontend_finish(frontend, NULL);
}

/** Candidate validation reuses the complete compiler pipeline after explicit
 * catalog preparation, including native definition admission before capture.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03),
 * [TPL-13](../../spec/language/templates.md#tpl-13).
 */
bool compiler_frontend_check_catalog(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, const char *const *required, size_t required_count) {
    assert(!frontend->attempted);
    frontend->attempted = true;
    if (project_load_catalog(&frontend->project, entry, required, required_count) != PROJECT_OK) {
        frontend->error = frontend->project.error;
        return false;
    }
    return compiler_frontend_admit(frontend) && compiler_frontend_capture(frontend, roots, count)
        && compiler_frontend_finish(frontend, NULL);
}
