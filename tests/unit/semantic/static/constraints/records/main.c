/** @file
 * @brief Check exact constraint input ownership, symmetric work keys and partial failures.
 */
#include <stdint.h>
#include "../common.c"

/** Mathematical duplicate pairs/requirements reuse records without allocation;
 * original application spans and derivation parents survive publication.
 */
static void it_retains_finite_equations_requirements_and_derivation_sites(void) {
    setup_constraints(NULL);
    SourceSpan site = constraint_module->first->site;
    assert(static_constraint_equate(&solver, left_variable, right_variable, site));
    const StaticConstraintEquation *parent = solver.first_equation;
    assert(parent && parent->left == left_variable && parent->right == right_variable);
    assert(parent->site.start == site.start && parent->site.source == site.source);
    size_t before = allocation_count;
    assert(static_constraint_equate(&solver, right_variable, left_variable, site));
    assert(allocation_count == before && solver.equation_count == 1);
    bool added = false;
    assert(static_constraint_pair(&solver, left_variable, plain_type, site, parent, &added) && added);
    assert(solver.last_equation->parent == parent && solver.equation_count == 2);
    assert(static_constraint_require(&solver, right_variable, site));
    before = allocation_count;
    assert(static_constraint_require(&solver, right_variable, site));
    assert(allocation_count == before && solver.requirement_count == 1);
    assert(solver.first_requirement->term == right_variable && solver.status == STATIC_CONSTRAINT_OK);
    assert(inventory.valid && term_store.status == STATIC_TERM_OK);
    destroy_constraints();
}

/** Refuse every observed publication allocation while preserving previous pairs
 * and requirements; the first error remains sticky without further allocation.
 */
static void it_unwinds_each_equation_and_requirement_record_refusal(void) {
    for (size_t operation = 0; operation < 2; operation++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_constraints(NULL);
            SourceSpan site = constraint_module->first->site;
            assert(static_constraint_equate(&solver, left_variable, right_variable, site));
            assert(static_constraint_require(&solver, left_variable, site));
            const StaticConstraintEquation *previous = solver.last_equation;
            const StaticConstraintRequirement *required = solver.last_requirement;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = operation ? static_constraint_require(&solver, right_variable, site) :
                static_constraint_equate(&solver, left_variable, plain_type, site);
            if (!failure) { assert(success); total = allocation_count - before; assert(total); }
            else {
                assert(!success && solver.status == STATIC_CONSTRAINT_OUT_OF_MEMORY);
                assert(solver.equation_count == 1 && previous->next == NULL);
                assert(solver.requirement_count == 1 && required->next == NULL);
                size_t after = allocation_count;
                assert(!static_constraint_equate(&solver, right_variable, plain_type, site));
                assert(allocation_count == after && diagnostic_valid(&solver.error));
            }
            failure_at = 0;
            destroy_constraints();
        }
    }
}

/** Copies/foreign handles cannot acquire identity; arithmetic refusal precedes
 * mutation and allocation and retains original application/definition sites.
 */
static void it_refuses_foreign_inputs_and_saturated_record_counts(void) {
    for (size_t partition = 0; partition < 4; partition++) {
        setup_constraints(NULL);
        StaticTerm copied = *left_variable;
        SourceSpan site = constraint_module->first->site;
        if (partition == 2) { solver.equation_count = SIZE_MAX; }
        if (partition == 3) { solver.requirement_count = SIZE_MAX; }
        size_t before = allocation_count;
        bool success;
        if (partition == 0) { success = static_constraint_equate(&solver, &copied, right_variable, site); }
        else if (partition == 1) { success = static_constraint_require(&solver, &copied, site); }
        else if (partition == 2) { success = static_constraint_equate(&solver, left_variable, right_variable, site); }
        else { success = static_constraint_require(&solver, left_variable, site); }
        assert(!success && allocation_count == before && solver.first_equation == NULL);
        assert(solver.first_requirement == NULL && diagnostic_valid(&solver.error));
        assert(solver.error.primary.span.start == site.start && solver.error.primary.span.source == site.source);
        assert(solver.status == (partition < 2 ? STATIC_CONSTRAINT_INVALID : STATIC_CONSTRAINT_OUT_OF_MEMORY));
        solver.equation_count = solver.requirement_count = 0;
        destroy_constraints();
    }
}

/** Execute independent finite-evidence and refusal partitions. */
static void run(void) {
    it_retains_finite_equations_requirements_and_derivation_sites();
    it_unwinds_each_equation_and_requirement_record_refusal();
    it_refuses_foreign_inputs_and_saturated_record_counts();
}
/** Return success after all assertions and counted teardown. */
int main(void) { run(); return 0; }
