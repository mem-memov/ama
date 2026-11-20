/** @file
 * @brief Prepare low-level ordinary checker fixtures from the actual guarded source/Type pipeline.
 * @see [T193 retirement contract](../../../docs/implementation/static-relocation.md)
 */
#ifndef AMA_SEMANTIC_FIXTURE_PREPARATION
#define AMA_SEMANTIC_FIXTURE_PREPARATION
#include "compiler/specializations/analysis.h"
#include "semantic/static/native/admission.h"
#include "semantic/specializations/inventory.h"

static StaticInventory fixture_inventory;
static StaticNativeAdmission fixture_native_admission;
static CompilerSpecialization fixture_specialization;
static bool fixture_inventory_initialized;
static bool fixture_specialization_initialized;

/** Keep actual upstream failure locations alive through fixture destruction; do not fabricate completed phase flags. */
static SemanticStatus fixture_failure(SemanticProgram *program, Diagnostic diagnostic) {
    program->error = diagnostic;
    program->status = diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY ? SEMANTIC_OUT_OF_MEMORY : SEMANTIC_INVALID;
    return program->status;
}
/** Register closed source-bound owners/fields for independent ordinary checker tests.
 * All source inference is the production compiler child operation, never a test replacement resolver. */
static SemanticStatus semantic_fixture_names(SemanticProgram *program) {
    assert(!fixture_inventory_initialized && !fixture_specialization_initialized);
    static_inventory_init(&fixture_inventory,program->project,sources.allocator);
    fixture_inventory_initialized = true;
    if (static_inventory_collect(&fixture_inventory) != STATIC_OK) { return fixture_failure(program,fixture_inventory.error); }
    if (!static_native_admit(&fixture_inventory,&fixture_native_admission)) {
        return fixture_failure(program,fixture_native_admission.error);
    }
    compiler_specialization_init(&fixture_specialization,&fixture_inventory,sources.allocator);
    fixture_specialization_initialized = true;
    const ProjectModule *roots[] = {program->project->entry};
    if (!compiler_specialization_check(&fixture_specialization,roots,program->project->entry ? 1 : 0)) {
        return fixture_failure(program,fixture_specialization.error);
    }
    return semantic_specialization_names(program,&fixture_specialization.closure);
}
/** Select the actual fixture entry by complete original module identity, independent of full-key enumeration order. */
static inline SemanticType *semantic_entry(void) {
    assert(project.entry);
    SemanticType *entry = semantic_find_type(&semantic,project.entry->source->logical_path);
    assert(entry && entry->module == project.entry);
    return entry;
}

/** Compose current prepared ordinary phases for planning fixtures; failed output is never consumed. */
static inline SemanticStatus semantic_fixture_check(SemanticProgram *program) {
    SemanticStatus status = semantic_fixture_names(program);
    if (status == SEMANTIC_OK) { status = semantic_native(program); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(program); }
    if (status == SEMANTIC_OK) { status = semantic_relations(program); }
    if (status == SEMANTIC_OK) { status = semantic_constants(program); }
    return status;
}

/** Destroy projected ordinary records before the borrowed closed source graph, then allow the next independent fixture. */
static void destroy_semantic_projection(void) {
    semantic_program_destroy(&semantic);
    if (fixture_specialization_initialized) { compiler_specialization_destroy(&fixture_specialization); }
    if (fixture_inventory_initialized) { static_inventory_destroy(&fixture_inventory); }
    fixture_specialization_initialized = fixture_inventory_initialized = false;
    fixture_native_admission = (StaticNativeAdmission){0};
}
#endif
