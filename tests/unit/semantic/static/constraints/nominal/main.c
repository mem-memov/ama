/** @file
 * @brief Check exact known-origin queries, unknown roots and allocation refusal.
 */
#include "../common.c"

/** Symbolic nominal children are valid metadata and do not close a required Type. */
static void it_reads_known_origins_without_closing_symbolic_captures(void) {
    setup_constraints(NULL);
    StaticTermCapture captures[] = {
        {.formal = static_symbol_at(constraint_module, STATIC_PARAMETER, 0), .value = left_variable},
        {.formal = static_symbol_at(constraint_module, STATIC_PARAMETER, 1), .value = left_variable}};
    const StaticTerm *nominal;
    assert(static_term_nominal(&term_store, constraint_module, captures, 2, (SourceSpan){0}, &nominal));
    assert(static_constraint_require(&solver, nominal, nominal->site));
    const StaticTerm *shape;
    StaticConstraintOutcome outcome;
    assert(static_constraint_nominal(&solver, nominal, &shape, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && shape == nominal && !shape->ground);
    const StaticTerm *value = NULL;
    assert(static_constraint_value(&solver, nominal, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_PENDING && value == NULL);
    assert(solver.equation_count == 0 && solver.requirement_count == 1);
    destroy_constraints();
}

/** An unknown root remains unknown until a declared equality proves its origin. */
static void it_follows_proven_aliases_without_guessing_an_unknown_constructor(void) {
    setup_constraints(NULL);
    const StaticTerm *shape = plain_type;
    StaticConstraintOutcome outcome;
    assert(static_constraint_nominal(&solver, left_variable, &shape, &outcome));
    assert(outcome == STATIC_CONSTRAINT_PENDING && shape == plain_type && solver.equation_count == 0);
    assert(static_constraint_equate(&solver, left_variable, right_variable, left_variable->site));
    assert(static_constraint_equate(&solver, right_variable, plain_type, right_variable->site));
    assert(static_constraint_nominal(&solver, left_variable, &shape, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && shape == plain_type);
    destroy_constraints();
}

/** Copied/absent term handles and missing receivers fail without modifying outputs. */
static void it_rejects_foreign_shapes_and_absent_query_receivers(void) {
    for (size_t kind = 0; kind < 4; kind++) {
        setup_constraints(NULL);
        StaticTerm copied = *plain_type;
        const StaticTerm *shape = plain_type;
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_COMPLETE;
        assert(!static_constraint_nominal(&solver, kind == 0 ? &copied : kind == 1 ? NULL : plain_type,
            kind == 2 ? NULL : &shape, kind == 3 ? NULL : &outcome));
        assert(shape == plain_type && outcome == STATIC_CONSTRAINT_COMPLETE && solver.status == STATIC_CONSTRAINT_INVALID);
        destroy_constraints();
    }
}

/** Query solving allocates only solver-owned scratch, including every failure prefix. */
static void it_unwinds_every_known_origin_query_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_constraints(NULL);
        assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        const StaticTerm *shape = NULL;
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
        bool success = static_constraint_nominal(&solver, left_variable, &shape, &outcome);
        if (!failure) { assert(success && shape == plain_type); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && shape == NULL && outcome == STATIC_CONSTRAINT_PENDING && solver.status == STATIC_CONSTRAINT_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_constraints();
    }
}

/** Exercise symbolic shape, alias evidence, handle validation and failure ownership. */
static void run(void) {
    it_reads_known_origins_without_closing_symbolic_captures();
    it_follows_proven_aliases_without_guessing_an_unknown_constructor();
    it_rejects_foreign_shapes_and_absent_query_receivers();
    it_unwinds_every_known_origin_query_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
