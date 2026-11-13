/**
 * @file
 * @brief Own the complete source, semantic and resolved-plan frontend graph.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Check once; source spans, proposed ABI and diagnostics stay valid until reverse destruction. No native compilation or execution occurs.
 */
#ifndef AMA_COMPILER_FRONTEND_H
#define AMA_COMPILER_FRONTEND_H

#include "planning/plan.h"
#include "project/inputs/inputs.h"

/**
 * Own project/source graph, exact input snapshot, optional authoring ABI, semantic graph and plan. Initialization
 * flags make every failed stage safely destructible; all exported views expire at destruction.
 */
typedef struct {
    Project project;
    ProjectInputs inputs;
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
/**
 * Initialize source/project owners and empty semantic/plan credits with the supplied allocator;
 * check once before destruction.
 */
void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator);
/**
 * Destroy initialized plan before semantic state, then proposed ABI sources and project storage;
 * reset the owner so borrowed outputs expire together.
 */
void compiler_frontend_destroy(CompilerFrontend *frontend);
/**
 * Load exactly once, then perform strict semantic/planning completion without authoring
 * overrides; false leaves safely destructible diagnostic state.
 */
bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry);

/**
 * Load an active native root and own a proposed ABI source if supplied or required for initial
 * authoring. Keep that source alive through semantic/plan destruction; executable checking uses
 * the separate strict path.
 */
bool compiler_frontend_authoring(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, const char *proposed_abi);

#endif
