/** @file
 * @brief Test finite type equations against independently stated alias and nominal laws.
 */
#include "../common.c"

/** Construct prepared nominal facts without applying source constructors; original
 * formals/captures remain independently visible to structural decomposition.
 */
static const StaticTerm *nominal(const StaticTerm *first, const StaticTerm *second, size_t count) {
    StaticTermCapture captures[] = {{.formal = constraint_module->first, .value = first},
        {.formal = constraint_module->first->next, .value = second}};
    const StaticTerm *term;
    assert(static_term_nominal(&term_store, constraint_module, captures, count,
        constraint_module->first->site, &term));
    return term;
}

/** Equality cycles add no origin; a later independently known anchor solves
 * both variables without changing either original scoped term identity.
 */
static void it_retains_unanchored_cycles_and_accepts_a_later_concrete_anchor(void) {
    setup_constraints(NULL);
    assert(static_constraint_equate(&solver, left_variable, right_variable, left_variable->site));
    assert(static_constraint_equate(&solver, right_variable, left_variable, right_variable->site));
    assert(static_constraint_require(&solver, left_variable, left_variable->site));
    StaticConstraintOutcome outcome = STATIC_CONSTRAINT_COMPLETE;
    assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
    assert(static_constraint_root(&solver, left_variable) == static_constraint_root(&solver, right_variable));
    assert(static_constraint_equate(&solver, right_variable, plain_type, right_variable->site));
    assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    assert(static_constraint_root(&solver, left_variable) == plain_type);
    assert(static_constraint_root(&solver, right_variable) == plain_type);
    assert(left_variable->kind == STATIC_TERM_VARIABLE && inventory.valid);
    size_t before = allocation_count;
    assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    assert(allocation_count == before);
    destroy_constraints();
}

/** Corresponding original captures impose equations, while residuals remain
 * local binders even when the required value is a partial constructor.
 */
static void it_decomposes_complete_and_partial_nominals_without_opening_residual_binders(void) {
    for (size_t count = 1; count <= 2; count++) {
        setup_constraints(NULL);
        const StaticTerm *symbolic = nominal(left_variable, plain_type, count);
        const StaticTerm *known = nominal(plain_type, plain_type, count);
        assert(static_constraint_equate(&solver, symbolic, known, left_variable->site));
        assert(static_constraint_require(&solver, symbolic, left_variable->site));
        StaticConstraintOutcome outcome;
        assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
        assert(static_constraint_root(&solver, left_variable) == plain_type);
        assert(solver.equation_count >= 2 && solver.last_equation->parent != NULL);
        assert(symbolic->as.nominal.residual_count == 2 - count);
        destroy_constraints();
    }
}

/** Different nominal origins conflict even when both have no runtime fields;
 * a previously anchored variable cannot silently replace its earlier solution.
 */
static void it_rejects_distinct_origins_and_conflicting_repeated_bindings(void) {
    for (size_t indirect = 0; indirect < 2; indirect++) {
        setup_constraints(NULL);
        const StaticTerm *other = nominal(plain_type, plain_type, 2);
        if (indirect) { assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site)); }
        assert(static_constraint_equate(&solver, indirect ? left_variable : plain_type, other, right_variable->site));
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
        assert(!static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
        assert(solver.status == STATIC_CONSTRAINT_CONFLICT && diagnostic_valid(&solver.error));
        assert(solver.error.primary.span.source == right_variable->site.source);
        assert(solver.error.primary.span.start == right_variable->site.start);
        assert(solver.error.related_count == 2);
        assert(strcmp(solver.error.related[0].name, "/test/fixture/v1.0.0/src/other") == 0);
        assert(strcmp(solver.error.related[1].name, "/test/fixture/v1.0.0/src/main") == 0);
        assert(solver.error.related[0].location.span.source == project_find(&project, "/test/fixture/v1.0.0/src/other")->source->text);
        assert(solver.error.related[1].location.span.source == project.entry->source->text);
        assert(solver.error.related[0].location.span.start == 0 && solver.error.related[1].location.span.start == 0);
        destroy_constraints();
    }
}

/** Direct and alias-mediated self-containment require an infinite substitution;
 * the finite source nominal graph itself is not rejected merely for its origin.
 */
static void it_rejects_direct_and_indirect_infinite_substitution_terms(void) {
    for (size_t indirect = 0; indirect < 2; indirect++) {
        setup_constraints(NULL);
        const StaticTerm *container = nominal(indirect ? right_variable : left_variable, plain_type, 2);
        assert(static_constraint_equate(&solver, left_variable, container, left_variable->site));
        if (indirect) { assert(static_constraint_equate(&solver, right_variable, left_variable, right_variable->site)); }
        StaticConstraintOutcome outcome = STATIC_CONSTRAINT_COMPLETE;
        assert(!static_constraint_solve(&solver, &outcome));
        assert(solver.status == STATIC_CONSTRAINT_CONFLICT && strstr(solver.error.message, "infinite") != NULL);
        assert(left_variable->kind == STATIC_TERM_VARIABLE && inventory.valid && term_store.status == STATIC_TERM_OK);
        destroy_constraints();
    }
}

/** Deferred representatives cannot erase known incompatible restrictions from
 * two other original variables in the same equivalence class.
 */
static void it_keeps_type_and_template_restrictions_through_a_deferred_alias(void) {
    setup_metadata_constraints();
    assert(left_variable->category == STATIC_CATEGORY_DEFERRED);
    const StaticTerm *family = constraint_symbol(".family");
    assert(static_constraint_describe(&solver, right_variable, STATIC_CATEGORY_TYPE, NULL, right_variable->site));
    assert(static_constraint_describe(&solver, family, STATIC_CATEGORY_TEMPLATE, constraint_actual_signature(), family->site));
    assert(static_constraint_equate(&solver, left_variable, right_variable, right_variable->site));
    assert(static_constraint_equate(&solver, left_variable, family, family->site));
    StaticConstraintOutcome outcome;
    assert(!static_constraint_solve(&solver, &outcome));
    assert(solver.status == STATIC_CONSTRAINT_CONFLICT && strstr(solver.error.message, "categories") != NULL);
    assert(solver.error.related_count == 2 && diagnostic_valid(&solver.error));
    destroy_constraints();
}

/** A concrete source module can reference its own nominal type; solving that
 * identity must not unfold its field definitions or mistake it for x = F(x).
 */
static void it_keeps_nominal_reference_cycles_outside_substitution_occurs_checking(void) {
    setup_constraints_with_sources(NULL, "@!\nlink&\n/test/fixture/v1.0.0/src/other\n");
    const ProjectModule *other = project_find(&project, "/test/fixture/v1.0.0/src/other");
    assert(other->syntax.body.first != NULL);
    assert(static_constraint_equate(&solver, left_variable, plain_type, left_variable->site));
    assert(static_constraint_require(&solver, left_variable, left_variable->site));
    assert(static_constraint_finish(&solver));
    assert(static_constraint_root(&solver, left_variable) == plain_type && inventory.valid);
    destroy_constraints();
}

/** Even unused captured static inputs remain part of nominal identity; a
 * different unused Type bindings cannot collapse into the same nominal identity.
 */
static void it_preserves_unused_capture_conflicts_without_erasing_values(void) {
    setup_constraints(NULL);
    const StaticTerm *first = nominal(plain_type, plain_type, 2);
    const StaticTerm *second = nominal(first, plain_type, 2);
    assert(static_constraint_equate(&solver, first, second, right_variable->site));
    StaticConstraintOutcome outcome;
    assert(!static_constraint_solve(&solver, &outcome) && solver.status == STATIC_CONSTRAINT_CONFLICT);
    destroy_constraints();


}

/** Normalization must retain both original sides of earlier equations, even
 * when their explanation graph is cyclic; rendering needs no new allocation. */
static void it_preserves_causal_binding_graphs_without_failure_allocations(void) {
    for (size_t order = 0; order < 2; order++) {
        setup_constraints(NULL);
        StaticConstraintOutcome outcome;
        if (order) {
            assert(static_constraint_equate(&solver, right_variable, plain_type, right_variable->site));
            assert(static_constraint_solve(&solver, &outcome));
        }
        assert(static_constraint_equate(&solver, left_variable, right_variable, left_variable->site));
        assert(static_constraint_solve(&solver, &outcome));
        if (!order) {
            assert(static_constraint_equate(&solver, right_variable, plain_type, right_variable->site));
            assert(static_constraint_solve(&solver, &outcome));
        }
        const StaticTerm *other = nominal(plain_type, plain_type, 2);
        assert(static_constraint_equate(&solver, left_variable, other, left_variable->site));
        size_t before = allocation_count;
        failure_at = before + 1;
        assert(!static_constraint_solve(&solver, &outcome));
        assert(allocation_count == before && solver.status == STATIC_CONSTRAINT_CONFLICT);
        assert(diagnostic_valid(&solver.error));
        bool left = false, right = false;
        size_t count = 0;
        for (const DiagnosticContext *node = solver.error.contexts; node; node = node->next) {
            assert(++count <= 4);
            if (strcmp(node->related.role, "original static variable definition") != 0) { continue; }
            left |= node->related.location.span.start == left_variable->site.start;
            right |= node->related.location.span.start == right_variable->site.start;
        }
        assert(left && right && count == 4);
        failure_at = 0;
        destroy_constraints();
    }
}

/** Execute separate finite-model success and contradiction partitions. */
static void run(void) {
    it_preserves_causal_binding_graphs_without_failure_allocations();
    it_retains_unanchored_cycles_and_accepts_a_later_concrete_anchor();
    it_decomposes_complete_and_partial_nominals_without_opening_residual_binders();
    it_rejects_distinct_origins_and_conflicting_repeated_bindings();
    it_rejects_direct_and_indirect_infinite_substitution_terms();
    it_keeps_type_and_template_restrictions_through_a_deferred_alias();
    it_keeps_nominal_reference_cycles_outside_substitution_occurs_checking();
    it_preserves_unused_capture_conflicts_without_erasing_values();
}
/** Return success only after every assertion and counted teardown. */
int main(void) { run(); return 0; }
