/** @file
 * @brief Test closure and metadata refinement without inventing constructor origins.
 */
#include "../common.c"

/** Declared category information alone is never a concrete type anchor; final
 * underdetermination is explicit and incremental pending queries preserve values.
 */
static void it_distinguishes_pending_queries_from_final_required_underdetermination(void) {
    setup_constraints(NULL);
    assert(static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_TYPE, NULL, left_variable->site));
    assert(static_constraint_require(&solver, left_variable, left_variable->site));
    const StaticTerm *value = plain_type;
    StaticConstraintOutcome outcome = STATIC_CONSTRAINT_COMPLETE;
    assert(static_constraint_value(&solver, left_variable, &value, &outcome));
    assert(value == plain_type && outcome == STATIC_CONSTRAINT_PENDING && solver.status == STATIC_CONSTRAINT_OK);
    assert(!static_constraint_finish(&solver));
    assert(solver.status == STATIC_CONSTRAINT_UNRESOLVED && diagnostic_valid(&solver.error));
    assert(solver.error.primary.span.source == left_variable->site.source);
    assert(solver.error.primary.span.start == left_variable->site.start);
    assert(!static_constraint_value(&solver, left_variable, &value, &outcome) && value == plain_type);
    destroy_constraints();
}

/** A partial constructor's local residuals are bound, but its captured Type
 * variables still need independent anchors. Solving does not mutate its payload.
 */
static void it_resumes_captured_type_closure_from_later_independent_equalities(void) {
    setup_constraints(NULL);
    StaticTermCapture capture = {.formal = static_symbol_at(constraint_module, STATIC_PARAMETER, 0), .value = left_variable};
    const StaticTerm *partial;
    assert(static_term_nominal(&term_store, constraint_module, &capture, 1, left_variable->site, &partial));
    assert(static_constraint_require(&solver, partial, partial->site));
    StaticConstraintOutcome outcome;
    assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
    assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
    assert(static_constraint_finish(&solver));
    const StaticTerm *closed = NULL;
    assert(static_constraint_value(&solver, partial, &closed, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    assert(closed != partial && closed->category == STATIC_CATEGORY_TEMPLATE && closed->ground);
    assert(closed->as.nominal.residual_count == 1 && closed->as.nominal.first_capture->value == plain_type);
    assert(partial->as.nominal.first_capture->value == left_variable && !partial->ground);
    assert(solver.metadata_count == 0 && inventory.valid);
    destroy_constraints();
}

/** A fully described constructor variable still has no nominal origin. A later
 * contradictory metadata fact is refused rather than overriding the first.
 */
static void it_keeps_metadata_evidence_separate_from_origin_evidence(void) {
    setup_metadata_constraints();
    assert(static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_TEMPLATE, constraint_actual_signature(), left_variable->site));
    assert(static_constraint_require(&solver, left_variable, left_variable->site));
    StaticConstraintOutcome outcome;
    assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
    assert(!static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_TYPE, NULL, right_variable->site));
    assert(solver.status == STATIC_CONSTRAINT_CONFLICT && solver.metadata_count == 1 && inventory.valid);
    destroy_constraints();
}

/** Diamond-shaped values have exponentially many paths but only 64 nominal
 * vertices. Substitution visits shared vertices once and returns the same
 * canonical value as independently constructing the known chain.
 */
static void it_normalizes_shared_deep_values_without_recursive_or_exponential_expansion(void) {
    setup_constraints(NULL);
    const StaticTerm *symbolic = left_variable, *expected = plain_type;
    for (size_t depth = 0; depth < 64; depth++) {
        StaticTermCapture captures[] = {{.formal = constraint_module->first, .value = symbolic},
            {.formal = constraint_module->first->next, .value = symbolic}};
        assert(static_term_nominal(&term_store, constraint_module, captures, 2, left_variable->site, &symbolic));
        captures[0].value = expected;
        captures[1].value = expected;
        assert(static_term_nominal(&term_store, constraint_module, captures, 2, left_variable->site, &expected));
    }
    assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
    assert(static_constraint_require(&solver, symbolic, left_variable->site));
    size_t before = allocation_count;
    const StaticTerm *value = NULL;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, symbolic, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == expected && value->ground);
    assert(allocation_count - before < 2000);
    size_t live = live_allocations;
    for (size_t repeat = 0; repeat < 100; repeat++) {
        assert(static_constraint_value(&solver, symbolic, &value, &outcome));
        assert(value == expected && outcome == STATIC_CONSTRAINT_COMPLETE && live_allocations == live);
    }
    assert(static_constraint_finish(&solver));
    destroy_constraints();
}

/** A substituted partial constructor stays Template: remaining original formals
 * are not missing required origins and preserve their complete named signature.
 */
static void it_normalizes_partial_results_and_retains_local_residuals(void) {
    setup_constraints(NULL);
    StaticTermCapture capture = {.formal = constraint_module->first, .value = left_variable};
    const StaticTerm *partial, *expected, *value = NULL;
    assert(static_term_nominal(&term_store, constraint_module, &capture, 1, left_variable->site, &partial));
    capture.value = plain_type;
    assert(static_term_nominal(&term_store, constraint_module, &capture, 1, left_variable->site, &expected));
    assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, partial, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == expected && value->ground);
    assert(value->category == STATIC_CATEGORY_TEMPLATE && value->signature->count == 1);
    assert(value->as.nominal.first_residual->formal == constraint_module->first->next);
    destroy_constraints();
}

/** Malformed prepared facts/output handles cannot become information or consume
 * source identity. Pending signatures must be resolved before being supplied.
 */
static void it_refuses_invalid_metadata_and_value_query_contracts(void) {
    for (size_t partition = 0; partition < 9; partition++) {
        setup_metadata_constraints();
        const StaticTerm *family = constraint_symbol(".family");
        const StaticSignature *known = constraint_actual_signature();
        StaticSignatureEntry unresolved = {.name = known->first->name, .category = STATIC_CATEGORY_DEFERRED};
        StaticSignature pending = {.first = &unresolved, .last = &unresolved, .count = 1};
        StaticTerm copied = *left_variable;
        StaticSignature empty = {0};
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
        const StaticTerm *value = plain_type;
        bool success = true;
        switch (partition) {
            case 0: success = static_constraint_describe(&solver, &copied, STATIC_CATEGORY_TYPE, NULL, left_variable->site); break;
            case 1: success = static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_DEFERRED, NULL, left_variable->site); break;
            case 2: success = static_constraint_describe(&solver, family, STATIC_CATEGORY_TEMPLATE, &empty, family->site); break;
            case 3: success = static_constraint_describe(&solver, family, STATIC_CATEGORY_TEMPLATE, NULL, family->site); break;
            case 4: success = static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_TYPE, known, left_variable->site); break;
            case 5: success = static_constraint_describe(&solver, family, STATIC_CATEGORY_TEMPLATE, &pending, family->site); break;
            case 6: success = static_constraint_value(&solver, left_variable, NULL, &outcome); break;
            case 7: success = static_constraint_value(&solver, left_variable, &value, NULL); break;
            case 8: success = static_constraint_solve(&solver, NULL); break;
        }
        assert(!success && solver.status == STATIC_CONSTRAINT_INVALID && solver.metadata_count == 0);
        assert(value == plain_type && outcome == STATIC_CONSTRAINT_PENDING && diagnostic_valid(&solver.error));
        assert(inventory.valid);
        destroy_constraints();
    }
}

/** Execute closure, metadata and iterative result partitions independently. */
static void run(void) {
    it_distinguishes_pending_queries_from_final_required_underdetermination();
    it_resumes_captured_type_closure_from_later_independent_equalities();
    it_keeps_metadata_evidence_separate_from_origin_evidence();
    it_normalizes_shared_deep_values_without_recursive_or_exponential_expansion();
    it_normalizes_partial_results_and_retains_local_residuals();
    it_refuses_invalid_metadata_and_value_query_contracts();
}
/** Return success after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
