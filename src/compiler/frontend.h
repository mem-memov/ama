/**
 * @file
 * @brief Own the complete source, semantic and resolved-plan frontend graph.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Check once; source spans, proposed ABI and diagnostics stay valid until reverse destruction. No native compilation or execution occurs.
 * [TPL-13](../../spec/language/templates.md#tpl-13) admits native own-formal
 * definitions before source/native input tracking or authoring preparation.
 */
#ifndef AMA_COMPILER_FRONTEND_H
#define AMA_COMPILER_FRONTEND_H

#include "planning/plan.h"
#include "project/inputs/inputs.h"
#include "semantic/static/native/admission.h"
#include "specializations/analysis.h"

/**
 * Own project/source graph, scoped static inventory/native admission, guarded Type closure, exact input
 * snapshot, optional authoring ABI, semantic graph and plan. Initialization
 * flags make every failed stage safely destructible; all exported views expire at destruction.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
typedef struct {
    Project project;
    ProjectInputs inputs;
    StaticInventory static_inventory;
    StaticNativeAdmission native_admission;
    CompilerSpecialization specializations;
    SemanticProgram semantic;
    ProgramPlan plan;
    Diagnostic error;
    SourceAllocator allocator;
    SourceManager proposed_sources;
    bool authoring;
    bool static_initialized;
    bool specialization_initialized;
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
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
void compiler_frontend_init(CompilerFrontend *frontend, SourceAllocator allocator);
/**
 * Destroy initialized plan before semantic state, then proposed ABI sources and project storage;
 * reset the owner so borrowed outputs expire together.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
void compiler_frontend_destroy(CompilerFrontend *frontend);
/**
 * Load exactly once, then perform strict semantic/planning completion without authoring
 * overrides; false leaves safely destructible diagnostic state.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_check(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry);
/** Check this owner's already built catalog through the ordinary required-module, input,
 * semantic and planning stages. Exact candidate source copies must be installed before the
 * call; no filesystem source rewriting or native execution occurs.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_check_catalog(CompilerFrontend *, const ModuleRoot *, size_t,
    const char *entry, const char *const *required, size_t required_count);

/** Analyze this owner's explicit captured catalog without an executable entry or
 * filesystem input capture. The caller owns source/native capture and version
 * authority; projected future companion paths are not read. Includes admission,
 * concrete-definition Type closure and ordinary checks; no executable storage plan
 * is built. Inputs remain
 * uncaptured/unsealed and cannot certify a build-cache or filesystem snapshot.
 * Success returns true with semantic/closure certificates; the composite valid
 * flag stays false because no executable plan exists.
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) requires
 * validation of prepared bytes before directory publication.
 */
bool compiler_frontend_check_snapshot(CompilerFrontend *, const char *const *, size_t);

/**
 * Load an active native root and own a proposed ABI source if supplied or required for initial
 * authoring. Keep that source alive through semantic/plan destruction; executable checking uses
 * the separate strict path.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_authoring(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry, const char *proposed_abi);

#endif
