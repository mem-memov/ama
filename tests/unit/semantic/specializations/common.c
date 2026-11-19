/** @file
 * @brief Own counted closed-Type and ordinary semantic fixtures in dependency destruction order.
 */
#ifndef AMA_SPECIALIZED_SEMANTICS_TEST_COMMON
#define AMA_SPECIALIZED_SEMANTICS_TEST_COMMON
#include "../static/common.c"
#include "compiler/specializations/analysis.h"
#include "semantic/specializations/inventory.h"
#include "semantic/internal.h"
#include "semantic/static/native/admission.h"

/** Independent owners preserve the source/Type/ordinary destruction order. */
static CompilerSpecialization analysis;
static SemanticProgram semantics;

/** Derive actual closed inputs through the production Type pipeline. */
static void setup(const char *main_text, const char *other_text, bool roots) {
    ModuleRoot directories[] = {{STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    setup_static_with_roots(main_text, other_text, directories, 2);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    StaticNativeAdmission native = {0};
    assert(static_native_admit(&inventory, &native));
    compiler_specialization_init(&analysis, &inventory, sources.allocator);
    const ProjectModule *selected[] = {project.entry};
    bool success = compiler_specialization_check(&analysis, roots ? selected : NULL, roots ? 1 : 0);
    if (!success) {
        fprintf(stderr, "Closure failure: %s at %zu..%zu\n", analysis.error.message,
            analysis.error.primary.span.start, analysis.error.primary.span.end);
    }
    assert(success);
    semantic_program_init(&semantics, &project, sources.allocator);
}

/** Release ordinary ABI/metadata before every borrowed closure/source object. */
static void destroy(void) {
    semantic_program_destroy(&semantics);
    compiler_specialization_destroy(&analysis);
    destroy_static();
}

/** Select fixture fields from their exact source name, without consulting inferred Types. */
static SemanticSymbol *field(SemanticType *owner, const char *name) {
    SemanticSymbol *result = semantic_symbol(owner, static_name_site(owner->module, name));
    assert(result && result->owner == owner && result->original);
    return result;
}

#endif
