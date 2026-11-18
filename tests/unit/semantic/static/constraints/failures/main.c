/** @file
 * @brief Verify solver failure ownership across mutable refinements and canonical result preparation.
 */
#include "../common.c"

/** Three symbolic levels force both solver scratch and new canonical term
 * allocations during result reconstruction, unlike an already ground input.
 */
static const StaticTerm *symbolic_value(void) {
    const StaticTerm *value = left_variable;
    for (size_t depth = 0; depth < 3; depth++) {
        StaticTermCapture captures[] = {{.formal = constraint_module->first, .value = value},
            {.formal = constraint_module->first->next, .value = right_variable}};
        assert(static_term_nominal(&term_store, constraint_module, captures, 2, left_variable->site, &value));
    }
    return value;
}

/** Exercise one operation without accepting any guessed result after refusal. */
static bool operation(size_t kind, const StaticTerm *value, const StaticTerm *known,
    const StaticTerm **output, StaticConstraintOutcome *outcome) {
    if (kind == 0 || kind == 1) { return static_constraint_solve(&solver, outcome); }
    if (kind == 2) {
        return static_constraint_describe(&solver, value, STATIC_CATEGORY_TEMPLATE, known->signature, value->site);
    }
    if (kind == 3 || kind == 6) { return static_constraint_value(&solver, value, output, outcome); }
    return static_constraint_finish(&solver);
}

/** Every observed allocation is refused once in each independently useful
 * operation. Failed solves may have refined mutable solver links; all term/source
 * payloads remain immutable and every partial owner is fully destructible.
 */
static void it_unwinds_every_solve_metadata_value_and_finalization_allocation_refusal(void) {
    for (size_t kind = 0; kind < 7; kind++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_constraints(kind == 2 ? "@<\n...a\n...b\n...inner\n ...value\n...known\n ...slot\n"
                "input\n...family\n ...slot < ...inner\n  ...value < ...a\n@!\n" : NULL);
            const StaticTerm *value = left_variable, *known = plain_type;
            if (kind == 2) { value = constraint_symbol("...family"); known = constraint_symbol("...known"); }
            else {
                value = symbolic_value();
                if (kind == 6) {
                    StaticTermCapture capture = {.formal = constraint_module->first, .value = left_variable};
                    assert(static_term_nominal(&term_store, constraint_module, &capture, 1, left_variable->site, &value));
                }
                assert(static_constraint_require(&solver, value, left_variable->site));
                if (kind != 5 && kind != 1) {
                    assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
                    assert(static_constraint_equate(&solver, right_variable, plain_type, right_variable->site));
                }
                if (kind == 1) {
                    StaticTermCapture captures[] = {{.formal = constraint_module->first, .value = plain_type},
                        {.formal = constraint_module->first->next, .value = plain_type}};
                    const StaticTerm *concrete;
                    assert(static_term_nominal(&term_store, constraint_module, captures, 2, left_variable->site, &concrete));
                    captures[0].value = left_variable;
                    captures[1].value = right_variable;
                    const StaticTerm *symbolic;
                    assert(static_term_nominal(&term_store, constraint_module, captures, 2, left_variable->site, &symbolic));
                    assert(static_constraint_equate(&solver, symbolic, concrete, right_variable->site));
                }
            }
            const StaticTerm *output = plain_type;
            StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = operation(kind, value, known, &output, &outcome);
            if (!failure) {
                assert(success == (kind != 5));
                if (kind == 5) { assert(solver.status == STATIC_CONSTRAINT_UNRESOLVED); }
                total = allocation_count - before;
                assert(total != 0);
            } else {
                assert(!success && solver.status == STATIC_CONSTRAINT_OUT_OF_MEMORY);
                assert(output == plain_type && outcome == STATIC_CONSTRAINT_PENDING);
                assert(left_variable->kind == STATIC_TERM_VARIABLE && right_variable->kind == STATIC_TERM_VARIABLE);
                assert(inventory.valid && inventory.status == STATIC_OK && diagnostic_valid(&solver.error));
                size_t after = allocation_count;
                const char *message = solver.error.message;
                assert(!static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
                assert(!static_constraint_finish(&solver) && allocation_count == after && solver.error.message == message);
            }
            failure_at = 0;
            destroy_constraints();
        }
    }
}

/** Counter saturation refuses before increment or publication, including a
 * generated capture equation and independently refined metadata record.
 */
static void it_refuses_saturated_variable_metadata_and_generated_equation_counts(void) {
    for (size_t kind = 0; kind < 5; kind++) {
        setup_constraints((kind == 1 || kind == 4) ? "@<\n...family\n ...slot\n...b\n@!\n...family\n ... > ...a\n" : NULL);
        if (kind == 3) {
            size_t before = allocation_count;
            assert(!static_constraint_lookup_capacity(&solver, SIZE_MAX));
            assert(allocation_count == before && solver.variables == NULL);
        } else if (kind == 4) {
            const StaticTerm *family = constraint_symbol("...family");
            solver.signature_count = SIZE_MAX;
            size_t before = allocation_count;
            assert(!static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_TEMPLATE, family->signature, left_variable->site));
            assert(allocation_count == before && solver.first_signature == NULL);
            solver.signature_count = 0;
        } else if (kind == 1) {
            solver.metadata_count = SIZE_MAX;
            size_t before = allocation_count;
            assert(!static_constraint_describe(&solver, left_variable, STATIC_CATEGORY_TYPE, NULL, left_variable->site));
            assert(allocation_count == before && solver.first_metadata == NULL);
            solver.metadata_count = 0;
        } else {
            if (kind == 0) {
                assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
                solver.variable_count = SIZE_MAX;
            } else {
                StaticTermCapture left[] = {{.formal = constraint_module->first, .value = left_variable}};
                StaticTermCapture right[] = {{.formal = constraint_module->first, .value = plain_type}};
                const StaticTerm *a, *b;
                assert(static_term_nominal(&term_store, constraint_module, left, 1, left_variable->site, &a));
                assert(static_term_nominal(&term_store, constraint_module, right, 1, right_variable->site, &b));
                assert(static_constraint_equate(&solver, a, b, left_variable->site));
                solver.equation_count = SIZE_MAX;
            }
            StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
            assert(!static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
            if (kind == 0) { solver.variable_count = 0; }
            else { solver.equation_count = 1; }
        }
        assert(solver.status == STATIC_CONSTRAINT_OUT_OF_MEMORY && inventory.valid && diagnostic_valid(&solver.error));
        destroy_constraints();
    }
}

/** Run every preparation-refusal and checked-arithmetic partition. */
static void run(void) {
    it_unwinds_every_solve_metadata_value_and_finalization_allocation_refusal();
    it_refuses_saturated_variable_metadata_and_generated_equation_counts();
}
/** Return success after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
