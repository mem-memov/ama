#include <assert.h>
#include "frontend.h"

void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator) {
    *frontend = (CompilerFrontend){.allocator = allocator};
    project_init(&frontend->project, allocator);
}

void compiler_frontend_destroy(CompilerFrontend *frontend) {
    if (frontend->plan_initialized) {
        program_plan_destroy(&frontend->plan);
    }
    if (frontend->semantic_initialized) {
        semantic_program_destroy(&frontend->semantic);
    }
    project_destroy(&frontend->project);
    *frontend = (CompilerFrontend){0};
}

bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries) {
    assert(!frontend->attempted);
    frontend->attempted = true;
    const char *roles[] = {libraries.action, libraries.index, libraries.condition, libraries.outcome};
    const char *required[4];
    size_t required_count = 0;
    for (size_t index = 0; index < 4; index++) {
        if (roles[index] != NULL) {
            required[required_count++] = roles[index];
        }
    }
    if (project_load_required(&frontend->project, roots, count, entry, required, required_count) != PROJECT_OK) {
        frontend->error = frontend->project.error;
        return false;
    }
    semantic_program_init(&frontend->semantic, &frontend->project, frontend->allocator);
    frontend->semantic_initialized = true;
    if (semantic_analyze(&frontend->semantic, libraries) != SEMANTIC_OK) {
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
