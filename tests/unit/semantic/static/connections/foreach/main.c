/** @file
 * @brief Check foreach Type links independently of runtime list contents.
 */
#include "../typed/common.c"

/** New and reused references receive the declared element Type even in empty bodies. */
static void it_links_owned_and_reference_list_elements_without_executing_bodies(void) {
    for (size_t references = 0; references < 2; references++) {
        setup_connections(references ? "@!\nvalues&[]\n//integer\nitem&\n//integer\nvalues&[item&]\n{\n}\n" :
            "@!\nvalues[]\n//integer\nvalues[item&]\n{\n}\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        const StaticTypedScope *scope;
        assert(static_connection_context(&connections, actual, &scope));
        bool changed;
        assert(static_connection_annotations(&connections, scope, &changed));
        assert(static_connection_foreach(&connections, actual, &changed));
        const StaticTypedField *binding;
        assert(static_connection_field(&connections, scope, scope->runtime->first_foreach->binding, &binding));
        const StaticTerm *value;
        StaticConstraintOutcome outcome;
        assert(static_constraint_value(&solver, binding->type, &value, &outcome));
        assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
        assert(!actual->registry_use->candidate->local_complete);
        assert(static_connection_foreach(&connections, actual, &changed) && !changed);
        destroy_connections();
    }
}

/** Unknown collection Types remain unknown until an independent concrete anchor arrives. */
static void it_refines_original_local_cells_without_rebinding_or_guessing(void) {
    setup_typed();
    const StaticTypedScope *metadata;
    assert(static_connection_scope(&connections, typed_environment(integer_type), &metadata));
    assert(typed_field(metadata, "values")->definition == metadata->runtime->first_foreach->collection);
    assert(results.context_count == 0);
    const StaticResultContext *actual = typed_actual();
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    bool changed;
    assert(static_connection_annotations(&connections, scope, &changed));
    assert(static_connection_foreach(&connections, actual, &changed) && changed);
    const StaticRuntimeForeach *loop = scope->runtime->first_foreach;
    const StaticTypedField *binding, *collection;
    assert(static_connection_field(&connections, scope, loop->binding, &binding));
    assert(static_connection_field(&connections, scope, loop->collection, &collection));
    const StaticTerm *value = NULL;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, binding->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_PENDING && value == NULL);
    assert(static_constraint_equate(&solver, collection->type, integer_type, loop->syntax->span));
    assert(static_constraint_value(&solver, binding->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(scope->runtime->first_foreach == loop && loop->binding == binding->definition);
    destroy_connections();
}

/** Existing scalar references must agree with collection Types; empty bodies are checked. */
static void it_rejects_conflicting_reused_reference_types(void) {
    setup_connections("@!\nvalues[]\n//integer\nitem&\n//text\nvalues[item&]\n{\n}\n", "@!\n");
    const StaticResultContext *actual = publication_context(caller_environment);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    bool changed;
    assert(static_connection_annotations(&connections, scope, &changed));
    assert(static_connection_foreach(&connections, actual, &changed));
    StaticConstraintOutcome outcome;
    assert(!static_constraint_solve(&solver, &outcome));
    assert(solver.status == STATIC_CONSTRAINT_CONFLICT);
    destroy_connections();
}

/** Foreign handles or absent receivers cannot introduce own-body equations. */
static void it_refuses_foreign_contexts_and_missing_progress_receivers(void) {
    for (size_t kind = 0; kind < 2; kind++) {
        setup_typed();
        const StaticResultContext *actual = typed_actual();
        StaticResultContext copied = *actual;
        bool changed = true;
        assert(!static_connection_foreach(&connections, kind ? actual : &copied, kind ? NULL : &changed));
        assert(changed && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** Sibling locals may share spelling while their different collections retain
 * distinct Types; assignments inside each body consume the retained local use.
 */
static void it_preserves_sibling_local_types_when_composed_with_assignments(void) {
    setup_connections("@!\nintegers[]\n//integer\ntexts[]\n//text\nintegers[item&]\n{\n first&\n  item&\n}\ntexts[item&]\n{\n second&\n  item&\n}\n", "@!\n");
    const StaticResultContext *actual = publication_context(caller_environment);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    bool changed;
    assert(static_connection_annotations(&connections, scope, &changed));
    assert(static_connection_foreach(&connections, actual, &changed));
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(outcome == STATIC_RESULT_READY);
    const StaticTypedField *first = typed_field(scope, "first"), *second = typed_field(scope, "second");
    const StaticTerm *left, *right;
    StaticConstraintOutcome closed;
    assert(static_constraint_value(&solver, first->type, &left, &closed));
    assert(closed == STATIC_CONSTRAINT_COMPLETE && left == integer_type);
    assert(static_constraint_value(&solver, second->type, &right, &closed));
    assert(closed == STATIC_CONSTRAINT_COMPLETE && right != left);
    const StaticRuntimeForeach *a = scope->runtime->first_foreach, *b = a->next;
    assert(a->binding != b->binding && a->binding->scope != b->binding->scope);
    destroy_connections();
}

/** Every observed context/field/equality allocation may fail with owned partial work. */
static void it_unwinds_every_foreach_equation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_typed();
        const StaticResultContext *actual = typed_actual();
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool changed = false;
        bool success = static_connection_foreach(&connections, actual, &changed);
        if (!failure) { assert(success && changed); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !changed && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_connections();
    }
}

/** Exercise Type inference, conflicts, identity refusal and allocation ownership. */
static void run(void) {
    it_links_owned_and_reference_list_elements_without_executing_bodies();
    it_refines_original_local_cells_without_rebinding_or_guessing();
    it_rejects_conflicting_reused_reference_types();
    it_refuses_foreign_contexts_and_missing_progress_receivers();
    it_preserves_sibling_local_types_when_composed_with_assignments();
    it_unwinds_every_foreach_equation_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
