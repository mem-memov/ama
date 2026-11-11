#include <assert.h>
#include "internal.h"

void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator) {
    *frontend = (CompilerFrontend){.allocator = allocator};
    project_init(&frontend->project, allocator);
    source_manager_init(&frontend->proposed_sources, allocator);
}

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

bool compiler_frontend_finish(CompilerFrontend *frontend, SemanticLibraries libraries,
    const SemanticNativeAuthoring *authoring) {
    semantic_program_init(&frontend->semantic, &frontend->project, frontend->allocator);
    frontend->semantic_initialized = true;
    if (semantic_analyze_authoring(&frontend->semantic, libraries, authoring) != SEMANTIC_OK) {
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

bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries) {
    return compiler_frontend_load(frontend, roots, count, entry, libraries)
        && compiler_frontend_finish(frontend, libraries, NULL);
}
