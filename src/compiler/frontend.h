#ifndef AMA_COMPILER_FRONTEND_H
#define AMA_COMPILER_FRONTEND_H

#include "planning/plan.h"

typedef struct {
    Project project;
    SemanticProgram semantic;
    ProgramPlan plan;
    Diagnostic error;
    SourceAllocator allocator;
    SourceManager proposed_sources;
    bool authoring;
    bool semantic_initialized;
    bool plan_initialized;
    bool attempted;
    bool valid;
} CompilerFrontend;

/* Own the complete frontend graph. Roots/options are borrowed for checking;
 * the diagnostic and all typed outputs survive until destruction. Check once.
 * No output files, C compiler invocations or native execution occur. */
void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator);
void compiler_frontend_destroy(CompilerFrontend *frontend);
bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries);

bool compiler_frontend_authoring(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, SemanticLibraries libraries, const char *proposed_abi);

#endif
