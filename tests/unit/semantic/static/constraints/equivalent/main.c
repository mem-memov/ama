/** @file
 * @brief Verify entailed symbolic equality without treating unknowns as different or introducing anchors.
 */
#include "../common.c"

/** All original captures, including unused ones, participate in nominal keys. */
static const StaticTerm *box(const StaticTerm *child, unsigned mask) {
    StaticTermCapture captures[3];
    size_t count = 0;
    for (size_t index = 0; index < 3; index++) {
        if (!(mask & (1u << index))) { continue; }
        captures[count++] = (StaticTermCapture){.formal = static_symbol_at(constraint_module, STATIC_PARAMETER, index), .value = child};
    }
    const StaticTerm *value;
    assert(static_term_nominal(&term_store, constraint_module, captures, count, (SourceSpan){0}, &value));
    return value;
}

/** Source variables and a concrete ordinary anchor have independently accepted metadata. */
static void setup(void) { setup_constraints("@<\nfirst\n...a\nsecond\n...b\nthird\n...c\n@!\n"); }

/** False means equality is not currently established: future independent facts
 * may make it true. A query never equates unknowns or closes their requirements.
 */
static void it_distinguishes_proven_equality_from_unknown_future_equal_completions(void) {
    setup();
    const StaticTerm *left = box(left_variable, 7), *right = box(right_variable, 7);
    bool equal = true;
    assert(static_constraint_equivalent(&solver, left, right, &equal) && !equal && solver.equation_count == 0);
    assert(static_constraint_equivalent(&solver, left_variable, left_variable, &equal) && equal);
    assert(static_constraint_equate(&solver, left_variable, right_variable, left_variable->site));
    assert(static_constraint_require(&solver, left_variable, left_variable->site));
    assert(static_constraint_equivalent(&solver, left, right, &equal) && equal);
    const StaticTerm *closed = NULL;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, left, &closed, &outcome) && outcome == STATIC_CONSTRAINT_PENDING && closed == NULL);
    assert(solver.equation_count == 1 && solver.requirement_count == 1);
    assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
    assert(static_constraint_equivalent(&solver, left, right, &equal) && equal);
    assert(static_constraint_equivalent(&solver, box(plain_type, 1), box(plain_type, 2), &equal) && !equal);
    assert(static_constraint_equivalent(&solver, box(plain_type, 1), box(plain_type, 7), &equal) && !equal);
    assert(static_constraint_equivalent(&solver, left, plain_type, &equal) && !equal);
    destroy_constraints();
}

/** Independent model: all five set partitions of three variables, six tagged
 * unary term families and every pair of their 16 concrete/symbolic members.
 * Tags encode nominal origin/partition/nesting; equality compares tags/classes,
 * independently of the production query's pair traversal and solver roots.
 */
static void it_matches_all_three_variable_partitions_and_tagged_nominal_family_pairs(void) {
    const unsigned classes[5][3] = {{0,1,2},{0,0,2},{0,1,0},{0,1,1},{0,0,0}};
    for (size_t partition = 0; partition < 5; partition++) {
        setup();
        const StaticTerm *variables[] = {left_variable, right_variable, constraint_symbol("...c")};
        const StaticTerm *terms[16] = {variables[0], variables[1], variables[2], plain_type};
        unsigned tags[16] = {0,0,0,1}, slots[16] = {0,1,2,0};
        for (size_t index = 0; index < 3; index++) {
            terms[4+index] = box(variables[index], 7);
            terms[7+index] = box(terms[4+index], 7);
            terms[10+index] = box(variables[index], 1);
            terms[13+index] = box(variables[index], 2);
            for (size_t family = 0; family < 4; family++) {
                tags[4+3*family+index] = (unsigned)(2+family);
                slots[4+3*family+index] = (unsigned)index;
            }
        }
        for (size_t left = 0; left < 3; left++) {
            for (size_t right = left+1; right < 3; right++) {
                if (classes[partition][left] == classes[partition][right]) {
                    assert(static_constraint_equate(&solver, variables[left], variables[right], variables[left]->site));
                }
            }
        }
        size_t equations = solver.equation_count;
        for (size_t left = 0; left < 16; left++) {
            for (size_t right = 0; right < 16; right++) {
                bool expected = tags[left] == tags[right] &&
                    (tags[left] == 1 || classes[partition][slots[left]] == classes[partition][slots[right]]);
                bool equal = !expected;
                assert(static_constraint_equivalent(&solver, terms[left], terms[right], &equal) && equal == expected);
                assert(solver.equation_count == equations && solver.requirement_count == 0);
            }
        }
        destroy_constraints();
    }
}

/** Shared children occur three times per level. A 1024-level DAG remains finite
 * and query scratch is released immediately instead of revisiting every path.
 */
static void it_compares_deep_shared_dags_iteratively_and_releases_query_scratch(void) {
    setup();
    const StaticTerm *left = left_variable, *right = right_variable;
    for (size_t depth = 0; depth < 1024; depth++) { left = box(left, 7); right = box(right, 7); }
    assert(static_constraint_equate(&solver, left_variable, right_variable, left_variable->site));
    StaticConstraintOutcome outcome;
    assert(static_constraint_solve(&solver, &outcome));
    size_t live = live_allocations;
    bool equal = false;
    assert(static_constraint_equivalent(&solver, left, right, &equal) && equal);
    assert(live_allocations == live && solver.equation_count == 1);
    destroy_constraints();
}

/** Foreign/copied terms and missing output are malformed, not merely unproved;
 * a later sticky error must not overwrite the original diagnostic or receiver.
 */
static void it_refuses_foreign_congruence_handles_and_preserves_failed_outputs(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup();
        StaticTerm copied = *left_variable;
        bool equal = true;
        assert(!static_constraint_equivalent(&solver, kind == 0 ? &copied : left_variable,
            kind == 1 ? NULL : right_variable, kind == 2 ? NULL : &equal));
        assert(equal && solver.status == STATIC_CONSTRAINT_INVALID && diagnostic_valid(&solver.error));
        const char *message = solver.error.message;
        assert(!static_constraint_equivalent(&solver, left_variable, right_variable, &equal) && equal && solver.error.message == message);
        destroy_constraints();
    }
}

/** Every actual query scratch allocation may fail. Persistent solver evidence
 * is prepared first; failed queries neither publish equality nor leak scratch.
 */
static void it_unwinds_every_symbolic_congruence_query_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup();
        const StaticTerm *left = left_variable, *right = right_variable;
        for (size_t depth = 0; depth < 4; depth++) { left = box(left, 7); right = box(right, 7); }
        assert(static_constraint_equate(&solver, left_variable, right_variable, left_variable->site));
        StaticConstraintOutcome outcome;
        assert(static_constraint_solve(&solver, &outcome));
        size_t before = allocation_count, live = live_allocations;
        if (cut) { failure_at = before + cut; }
        bool equal = false;
        bool success = static_constraint_equivalent(&solver, left, right, &equal);
        if (!cut) { assert(success && equal); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !equal && solver.status == STATIC_CONSTRAINT_OUT_OF_MEMORY && diagnostic_valid(&solver.error)); }
        failure_at = 0;
        assert(live_allocations == live && solver.equation_count == 1);
        destroy_constraints();
    }
}

/** Run proof/unknown controls, finite model, deep DAG and refusal partitions. */
static void run(void) {
    it_distinguishes_proven_equality_from_unknown_future_equal_completions();
    it_matches_all_three_variable_partitions_and_tagged_nominal_family_pairs();
    it_compares_deep_shared_dags_iteratively_and_releases_query_scratch();
    it_refuses_foreign_congruence_handles_and_preserves_failed_outputs();
    it_unwinds_every_symbolic_congruence_query_allocation_refusal();
}
/** Return only after all assertions and counted teardown. */
int main(void) { run(); return 0; }
