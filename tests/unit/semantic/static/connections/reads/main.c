/** @file
 * @brief Check explicit constant reads independently of runtime value construction.
 */
#include "../operations/common.c"

/** Named and positional shared reads infer scalar/list receivers from own declarations. */
static void it_connects_named_and_positional_shared_results_after_namespace_validation(void) {
    for (size_t named = 0; named < 2; named++) {
        setup_connections(named ? "@!\n.other\n value >> scalar\n items[] >> values[]\n" :
            "@!\n.other\n >> scalar\n >> values[]\n", "@=\nvalue\n..integer\nitems[]\n..integer\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const AstNode *node = operation_at(scope, 0);
        bool changed;
        StaticResultOutcome outcome;
        assert(static_connection_read(&connections, actual, node, NULL, &changed, &outcome));
        assert(outcome == STATIC_RESULT_PENDING && !changed && constants.read_count == 1 && constants.selection_count == 0);
        const StaticResultContext *supplier = constant_fixed_target_context();
        assert(static_connection_read(&connections, actual, node, supplier, &changed, &outcome));
        assert(changed && outcome == STATIC_RESULT_READY);
        for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
            const StaticTerm *value;
            StaticConstraintOutcome closed;
            assert(static_constraint_value(&solver, field->type, &value, &closed));
            assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
        }
        assert(static_connection_read(&connections, actual, node, supplier, &changed, &outcome));
        assert(!changed && outcome == STATIC_RESULT_READY && !actual->registry_use->candidate->local_complete);
        destroy_connections();
    }
}

/** A namespace parameter cannot be supplied by its saved receiver's Type. */
static void it_keeps_open_read_namespaces_independent_of_concrete_receivers(void) {
    setup_connections("@!\nanchor\n..integer\n.other\n value >> anchor\n",
        "@=\nvalue\n...slot\n@<\nargument\n...slot\n@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_read(&connections, actual, operation_at(scope, 0), NULL, &changed, &outcome));
    assert(outcome == STATIC_RESULT_PENDING && constants.selection_count == 0);
    assert(constants.first_read->source->value != integer_type);
    destroy_connections();
}

/** Scalar/list/reference receiver forms must match the independently selected constant. */
static void it_rejects_incompatible_saved_constant_receiver_storage(void) {
    setup_connections("@!\n.other\n value >> receiver&\n", "@=\nvalue\n..integer\n@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    const StaticResultContext *supplier = constant_fixed_target_context();
    bool changed = true;
    StaticResultOutcome outcome = STATIC_RESULT_READY;
    assert(!static_connection_read(&connections, actual, operation_at(scope, 0), supplier, &changed, &outcome));
    assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_CONFLICT);
    destroy_connections();
}

/** Copied syntax, wrong namespace contexts and missing outputs cannot certify reads. */
static void it_refuses_foreign_read_syntax_and_invalid_output_or_namespace_handles(void) {
    for (size_t kind = 0; kind < 4; kind++) {
        setup_connections("@!\n.other\n value >> receiver\n", "@=\nvalue\n..integer\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const AstNode *node = operation_at(scope, 0);
        AstNode copied = *node;
        const StaticResultContext *supplier = constant_fixed_target_context();
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_read(&connections, actual, kind == 0 ? &copied : node,
            kind == 1 ? actual : supplier, kind == 2 ? NULL : &changed, kind == 3 ? NULL : &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** Every original read registration, selection and equality allocation may refuse. */
static void it_unwinds_every_explicit_read_equation_allocation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_connections("@!\n.other\n value >> receiver\n", "@=\nvalue\n..integer\n@!\n");
            const StaticResultContext *actual;
            const StaticTypedScope *scope = prepare_operations(&actual);
            const AstNode *node = operation_at(scope, 0);
            const StaticResultContext *supplier = phase ? constant_fixed_target_context() : NULL;
            bool changed;
            StaticResultOutcome outcome;
            if (phase) { assert(static_connection_read(&connections, actual, node, NULL, &changed, &outcome)); }
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            changed = false;
            outcome = STATIC_RESULT_PENDING;
            bool success = static_connection_read(&connections, actual, node, supplier, &changed, &outcome);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && !changed && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_connections();
        }
    }
}

/** Exercise saved read ordering, independent namespaces, exact forms and refusals. */
static void run(void) {
    it_connects_named_and_positional_shared_results_after_namespace_validation();
    it_keeps_open_read_namespaces_independent_of_concrete_receivers();
    it_rejects_incompatible_saved_constant_receiver_storage();
    it_refuses_foreign_read_syntax_and_invalid_output_or_namespace_handles();
    it_unwinds_every_explicit_read_equation_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
