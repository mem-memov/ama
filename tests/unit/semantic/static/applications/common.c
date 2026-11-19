/** @file
 * @brief Own actual source, term, solution and application owners in their declared borrow order.
 */
#ifndef AMA_STATIC_APPLICATION_TEST_COMMON
#define AMA_STATIC_APPLICATION_TEST_COMMON
#include <stdint.h>
#include "../common.c"
#include "semantic/static/applications/internal.h"

/** Every owner expires before the source/allocator it borrows. */
static BindingPlan binding_plan;
static StaticTermStore terms;
static StaticConstraintSolver solver;
static StaticApplicationPlan applications;
static const StaticModule *caller_module;
static const StaticModule *target_module;
static const StaticTermEnvironment *caller_environment;
static const StaticTerm *caller_constructor;
static const StaticTerm *target_constructor;
static const StaticTerm *integer_type;

/** Complete scoped input names retain original identity regardless of collector
 * insertion order. Optional consumers use original ordinals directly.
 */
static inline const StaticSymbol *application_formal(const StaticModule *module, size_t ordinal) {
    const StaticSymbol *formal = static_symbol_at(module, STATIC_PARAMETER, ordinal);
    assert(formal != NULL);
    return formal;
}

/** Load real standard integer metadata through a reference field; it never
 * executes a native body or invents an arbitrary default Type for a fixture.
 */
static void setup_applications(const char *caller, const char *target) {
    ModuleRoot roots[] = {{STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT},
        {"../../../../../../stdlib/modules", NULL, 0, MODULE_ROOT_STANDARD}};
    setup_static_with_roots(caller ? caller : "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\nanchor&\n..integer\n",
        target ? target : "@<\nfirst argument\n...a\nsecond argument\n...b\n@!\n", roots, 2);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    caller_module = static_module_find(&inventory, project.entry);
    target_module = static_module_find(&inventory, project_find(&project, ".other"));
    binding_plan_init(&binding_plan, sources.allocator);
    static_term_store_init(&terms, &inventory, sources.allocator);
    assert(static_term_environment(&terms, caller_module, NULL, &caller_environment));
    caller_constructor = NULL;
    if (caller_module->definition->syntax.sections.kind != MODULE_STATIC) {
        assert(static_term_nominal(&terms, caller_module, NULL, 0, (SourceSpan){0}, &caller_constructor));
    }
    target_constructor = NULL;
    if (target_module->definition->syntax.sections.kind != MODULE_STATIC) {
        assert(static_term_nominal(&terms, target_module, NULL, 0, (SourceSpan){0}, &target_constructor));
    }
    const StaticModule *integer = static_module_find(&inventory, project_find(&project, "..integer"));
    assert(integer != NULL && integer->parameter_count == 0);
    assert(static_term_nominal(&terms, integer, NULL, 0, (SourceSpan){0}, &integer_type));
    static_constraint_init(&solver, &terms, sources.allocator);
    static_application_plan_init(&applications, &terms, &solver, &binding_plan, sources.allocator);
}

/** Source parameter queries are optional fixture operations, never name search
 * across another template's foreign signature.
 */
static inline const StaticTerm *application_symbol(const char *name) {
    const StaticSymbol *symbol = static_symbol_find(caller_module, static_name_site(project.entry, name));
    assert(symbol != NULL);
    const StaticTerm *value;
    assert(static_term_symbol(&terms, caller_environment, symbol, &value));
    return value;
}

/** Destroy application snapshots before their solver, values, requests and source
 * definitions; counted allocator teardown verifies every refused preparation.
 */
static void destroy_applications(void) {
    static_application_plan_destroy(&applications);
    static_constraint_destroy(&solver);
    static_term_store_destroy(&terms);
    binding_plan_destroy(&binding_plan);
    destroy_static();
}
#endif
