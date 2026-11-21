/** @file
 * @brief Check ordinary call constraints without expanding or executing callees.
 */
#include "../operations/common.c"

/** Independent ordinary connections may determine a generic callee from either direction. */
static void it_infers_generic_callee_types_from_inputs_and_existing_output_anchors(void) {
    for (size_t direction = 0; direction < 2; direction++) {
        setup_connections(direction ? "@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n > anchor\n" :
            "@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < anchor\n > fresh\n",
            "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const AstNode *node = operation_at(scope, 0);
        bool changed;
        StaticResultOutcome outcome;
        assert(static_connection_bindings(&connections, actual, node, &changed, &outcome));
        assert(changed && outcome == STATIC_RESULT_READY);
        const StaticTypedOperation *operation;
        assert(static_connection_operation(&connections, actual, node, &operation, &outcome));
        for (const StaticTypedField *field = operation->target->first_field; field; field = field->next) {
            const StaticTerm *value;
            StaticConstraintOutcome closed;
            assert(static_constraint_value(&solver, field->type, &value, &closed));
            assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
        }
        assert(results.context_count == 1 && !actual->registry_use->candidate->local_complete);
        assert(static_connection_bindings(&connections, actual, node, &changed, &outcome) && !changed);
        destroy_connections();
    }
}

/** Named requests and positional skips resolve the same original declaration identities. */
static void it_preserves_named_fields_and_independent_positional_skip_order(void) {
    for (size_t named = 0; named < 2; named++) {
        setup_connections(named ? "@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n second < anchor\n returned > fresh\n" :
            "@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n <\n < anchor\n > fresh\n",
            "@<\nfirst\n//text\nsecond\n.slot\n@>\nreturned\n.slot\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        bool changed;
        StaticResultOutcome outcome;
        assert(static_connection_bindings(&connections, actual, operation_at(scope, 0), &changed, &outcome));
        assert(changed && outcome == STATIC_RESULT_READY);
        const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, "fresh"));
        const StaticTypedField *field;
        assert(static_connection_field(&connections, scope, definition, &field));
        const StaticTerm *value;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, field->type, &value, &closed));
        assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
        destroy_connections();
    }
}

/** A literal contributes capability work but no inferred native Type. */
static void it_preserves_literal_input_requirements_without_inventing_an_anchor(void) {
    setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < |123\n > fresh\n",
        "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_bindings(&connections, actual, operation_at(scope, 0), &changed, &outcome));
    assert(outcome == STATIC_RESULT_PENDING);
    const StaticTypedField *field;
    assert(static_connection_field(&connections, scope,
        static_runtime_field(scope->runtime, static_name_site(project.entry, "fresh")), &field));
    const StaticTerm *value = NULL;
    StaticConstraintOutcome closed;
    assert(static_constraint_value(&solver, field->type, &value, &closed));
    assert(closed == STATIC_CONSTRAINT_PENDING && value == NULL);
    destroy_connections();
}

/** Input and output storage mismatches cannot be repaired by Type unification. */
static void it_rejects_implicit_reference_conversion_in_both_binding_directions(void) {
    for (size_t output = 0; output < 2; output++) {
        setup_connections(output ? "@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n > fresh&\n" :
            "@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < & anchor\n",
            "@<\nargument\n//integer\n@>\nreturned\n//integer\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_bindings(&connections, actual, operation_at(scope, 0), &changed, &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_CONFLICT);
        destroy_connections();
    }
}

/** Finite repeated template nesting is caller-supplied composition, not recursive
 * body expansion. Each wrapper's retained result exposes the original inner Type.
 */
static void it_composes_two_and_three_nested_owner_interfaces_without_body_expansion(void) {
    const char *sources[] = {
        "@!\nleaf\n//integer\ninner\n/test/fixture/v1.0.0/src/other\ninner !\n < leaf\nouter\n/test/fixture/v1.0.0/src/other\nouter !\n < inner\n > carried\ncarried !\n > final\n",
        "@!\nleaf\n//integer\ninner\n/test/fixture/v1.0.0/src/other\ninner !\n < leaf\nouter\n/test/fixture/v1.0.0/src/other\nouter !\n < inner\ntop\n/test/fixture/v1.0.0/src/other\ntop !\n < outer\n > carried\ncarried !\n > middle\nmiddle !\n > final\n"
    };
    for (size_t depth = 0; depth < 2; depth++) {
        setup_connections(sources[depth], "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        size_t contexts = results.context_count;
        for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
            if (event->kind != STATIC_RUNTIME_CALL) { continue; }
            bool changed;
            StaticResultOutcome outcome;
            assert(static_connection_bindings(&connections, actual, event->node, &changed, &outcome));
            assert(outcome == STATIC_RESULT_READY);
        }
        const StaticTypedField *field;
        assert(static_connection_field(&connections, scope,
            static_runtime_field(scope->runtime, static_name_site(project.entry, "final")), &field));
        const StaticTerm *value;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, field->type, &value, &closed));
        assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
        assert(results.context_count == contexts && !actual->registry_use->candidate->local_complete);
        destroy_connections();
    }
}

/** Distinct field applications may specialize differently; proven identical
 * owners must agree on their original body-field Types even before body checks.
 */
static void it_separates_independent_field_applications_and_joins_only_proven_equal_owners(void) {
    for (size_t same = 0; same < 2; same++) {
        setup_connections(same ? "@!\nnumber\n//integer\ntext\n//text\nfirst\n/test/fixture/v1.0.0/src/other\nsecond\n/test/fixture/v1.0.0/src/other\nfirst !\n < number\nsecond !\n < number\n" :
            "@!\nnumber\n//integer\ntext\n//text\nfirst\n/test/fixture/v1.0.0/src/other\nsecond\n/test/fixture/v1.0.0/src/other\nfirst !\n < number\nsecond !\n < text\n",
            "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\nlocal\n argument\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        bool changed;
        StaticResultOutcome outcome;
        for (size_t i = 0; i < 2; i++) {
            assert(static_connection_bindings(&connections, actual, operation_at(scope, i), &changed, &outcome));
            assert(outcome == STATIC_RESULT_READY);
        }
        const StaticTypedOperation *a = connections.first_operation, *b = a->next;
        assert(a->target != b->target && a->target->runtime == b->target->runtime);
        const StaticTypedField *left = a->target->last_field, *right = b->target->last_field;
        assert(left->definition == right->definition && left->type != right->type);
        assert(static_connection_merge_fields(&connections, &changed));
        bool equal;
        assert(static_constraint_equivalent(&solver, left->type, right->type, &equal));
        assert(equal == (same != 0));
        const StaticTerm *owner_a, *owner_b;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, a->target->owner, &owner_a, &closed) && closed == STATIC_CONSTRAINT_COMPLETE);
        assert(static_constraint_value(&solver, b->target->owner, &owner_b, &closed) && closed == STATIC_CONSTRAINT_COMPLETE);
        assert((owner_a == owner_b) == (same != 0));
        destroy_connections();
    }
}

/** Missing progress/outcome receivers leave caller-visible values untouched. */
static void it_refuses_missing_binding_outputs_before_submitting_equations(void) {
    for (size_t kind = 0; kind < 2; kind++) {
        setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n", "@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_bindings(&connections, actual, operation_at(scope, 0), kind ? &changed : NULL, kind ? NULL : &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** All interface, operand and equality allocation failures retain owned partial work. */
static void it_unwinds_every_binding_equation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < anchor\n > fresh\n",
            "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const AstNode *node = operation_at(scope, 0);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool changed = false;
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        bool success = static_connection_bindings(&connections, actual, node, &changed, &outcome);
        if (!failure) { assert(success && changed); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !changed && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_connections();
    }
}

/** Exercise bidirectional inference, request ordering, pending evidence and failures. */
static void run(void) {
    it_infers_generic_callee_types_from_inputs_and_existing_output_anchors();
    it_preserves_named_fields_and_independent_positional_skip_order();
    it_preserves_literal_input_requirements_without_inventing_an_anchor();
    it_rejects_implicit_reference_conversion_in_both_binding_directions();
    it_composes_two_and_three_nested_owner_interfaces_without_body_expansion();
    it_separates_independent_field_applications_and_joins_only_proven_equal_owners();
    it_refuses_missing_binding_outputs_before_submitting_equations();
    it_unwinds_every_binding_equation_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
