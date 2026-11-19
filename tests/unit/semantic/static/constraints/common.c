/** @file
 * @brief Own counted finite constraint fixtures over actual scoped source records.
 */
#ifndef AMA_CONSTRAINT_TEST_COMMON
#define AMA_CONSTRAINT_TEST_COMMON
#include <stdint.h>
#include "../common.c"
#include "semantic/static/constraints/internal.h"

/** Solver expires before terms, accepted inventory and loaded sources. */
static StaticTermStore term_store;
static StaticConstraintSolver solver;
static const StaticModule *constraint_module;
static const StaticTermEnvironment *constraint_environment;
static const StaticTerm *left_variable;
static const StaticTerm *right_variable;
static const StaticTerm *plain_type;

/** Look up complete original source names; helpers never infer another scope. */
static inline const StaticTerm *constraint_symbol(const char *name) {
    const StaticSymbol *symbol = static_symbol_find(constraint_module,
        static_name_site(project.entry, name));
    assert(symbol != NULL);
    const StaticTerm *value;
    assert(static_term_symbol(&term_store, constraint_environment, symbol, &value));
    return value;
}

/** Prepare source identities and terms before testing the independent solver. */
static void setup_constraints_with_sources(const char *text, const char *other) {
    setup_static(text != NULL ? text : "@<\n...a\n...b\n@!\n", other);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    constraint_module = static_module_find(&inventory, project.entry);
    static_term_store_init(&term_store, &inventory, sources.allocator);
    assert(static_term_environment(&term_store, constraint_module, NULL, &constraint_environment));
    left_variable = constraint_symbol("...a");
    right_variable = constraint_symbol("...b");
    assert(static_term_nominal(&term_store, static_module_find(&inventory, project_find(&project, ".other")),
        NULL, 0, (SourceSpan){0}, &plain_type));
    size_t before = allocation_count;
    static_constraint_init(&solver, &term_store, sources.allocator);
    assert(allocation_count == before);
}

/** Default concrete anchor has no fields or free static parameters. */
static void setup_constraints(const char *text) {
    setup_constraints_with_sources(text, "@!\n");
}

/** Release every failed/partial owner and assert counted full source teardown. */
static void destroy_constraints(void) {
    static_constraint_destroy(&solver);
    assert(solver.terms == NULL && solver.first_equation == NULL && solver.first_requirement == NULL);
    static_term_store_destroy(&term_store);
    destroy_static();
}
#endif
