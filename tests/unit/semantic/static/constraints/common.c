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
    setup_static(text != NULL ? text : "@<\nfirst argument\n.a\nsecond argument\n.b\n@!\n", other);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    constraint_module = static_module_find(&inventory, project.entry);
    static_term_store_init(&term_store, &inventory, sources.allocator);
    assert(static_term_environment(&term_store, constraint_module, NULL, &constraint_environment));
    left_variable = constraint_symbol(".a");
    right_variable = constraint_symbol(".b");
    assert(static_term_nominal(&term_store, static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/other")),
        NULL, 0, (SourceSpan){0}, &plain_type));
    size_t before = allocation_count;
    static_constraint_init(&solver, &term_store, sources.allocator);
    assert(allocation_count == before);
}

/** Default concrete anchor has no fields or free static parameters. */
static void setup_constraints(const char *text) {
    setup_constraints_with_sources(text, "@!\n");
}

/** Free graph values have no preselected category or closed source signature. */
#define CONSTRAINT_METADATA_SOURCE "@...\n.first\n.a\n.second\n.b\n.third\n.family\n.fourth\n.known\n"

/** Prepare a real one-slot ordinary constructor as independent metadata evidence. */
static inline void setup_metadata_constraints(void) {
    setup_constraints_with_sources(CONSTRAINT_METADATA_SOURCE, "@<\nargument\n.slot\n@!\n");
}

/** Borrow only the actual signature, deliberately not equating its nominal
 * origin to another variable. Even a unique matching source cannot fill that gap.
 */
static inline const StaticSignature *constraint_actual_signature(void) {
    assert(plain_type->category == STATIC_CATEGORY_TEMPLATE && plain_type->signature != NULL);
    const StaticSignature *signature = plain_type->signature;
    assert(signature->count == 1 && signature->first->category == STATIC_CATEGORY_TYPE);
    assert(signature->first->signature == NULL && layout_text_is(signature->first->name, ".slot"));
    assert(signature->first->name.source != left_variable->site.source);
    return signature;
}

/** Release every failed/partial owner and assert counted full source teardown. */
static void destroy_constraints(void) {
    static_constraint_destroy(&solver);
    assert(solver.terms == NULL && solver.first_equation == NULL && solver.first_requirement == NULL);
    static_term_store_destroy(&term_store);
    destroy_static();
}
#endif
