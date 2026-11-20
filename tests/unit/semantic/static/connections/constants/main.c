/** @file
 * @brief Check qualified constants without receiver-driven namespace inference.
 */
#include "../operations/common.c"

/** Independently checked namespaces refine inferred receivers without value execution. */
static void it_resolves_qualified_assignment_types_only_after_explicit_namespace_validation(void) {
    setup_connections("@!\nreceived\n /test/fixture/v1.0.0/src/other/value\n", "@=\nvalue\n//integer\n |1\n@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(!changed && outcome == STATIC_RESULT_PENDING && connections.constant_count == 1 && results.context_count == 1);
    const StaticTypedConstant *transfer = connections.first_constant;
    assert(transfer->source->namespace_type == target_constructor && !transfer->equations_ready);
    const StaticResultContext *supplier = constant_fixed_target_context();
    assert(static_connection_constant(&connections, transfer, supplier, &changed) && changed);
    const StaticTerm *value;
    StaticConstraintOutcome closed;
    assert(static_constraint_value(&solver, transfer->type, &value, &closed));
    assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type && transfer->equations_ready);
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(!changed && outcome == STATIC_RESULT_READY && connections.constant_count == 1);
    assert(static_connection_constant(&connections, transfer, supplier, &changed) && !changed);
    assert(scope->field_count == 1 && !actual->registry_use->candidate->local_complete);
    destroy_connections();
}

/** Initializer and ordinary input consumers retain the same independent namespace boundary. */
static void it_connects_qualified_list_element_initializers_and_call_inputs(void) {
    for (size_t call = 0; call < 2; call++) {
        setup_connections(call ? "@!\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < /test/fixture/v1.0.0/src/other/value\n" :
            "@!\nvalues[]\n//integer\n /test/fixture/v1.0.0/src/other/value\n",
            "@=\nvalue\n//integer\n@<\nargument\n//integer\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        bool changed;
        StaticResultOutcome outcome;
        bool success = call ? static_connection_bindings(&connections, actual, operation_at(scope, 0), &changed, &outcome) :
            static_connection_initializers(&connections, actual, &changed, &outcome);
        assert(success && outcome == STATIC_RESULT_PENDING && connections.constant_count == 1);
        const StaticResultContext *supplier = constant_fixed_target_context();
        assert(static_connection_constant(&connections, connections.first_constant, supplier, &changed));
        success = call ? static_connection_bindings(&connections, actual, operation_at(scope, 0), &changed, &outcome) :
            static_connection_initializers(&connections, actual, &changed, &outcome);
        assert(success && outcome == STATIC_RESULT_READY && connections.constant_count == 1);
        destroy_connections();
    }
}

/** Owned constant values cannot implicitly form references or splice whole lists. */
static void it_rejects_reference_and_scalar_list_storage_mismatches(void) {
    for (size_t list = 0; list < 2; list++) {
        setup_connections(list ? "@!\nreceived\n /test/fixture/v1.0.0/src/other/items[]\n" : "@!\nreceived&\n /test/fixture/v1.0.0/src/other/value\n",
            "@=\nvalue\n//integer\nitems[]\n//integer\n@!\n");
        const StaticResultContext *actual;
        (void)prepare_operations(&actual);
        bool changed;
        StaticResultOutcome outcome;
        assert(static_connection_assignments(&connections, actual, &changed, &outcome));
        const StaticResultContext *supplier = constant_fixed_target_context();
        changed = true;
        assert(!static_connection_constant(&connections, connections.first_constant, supplier, &changed));
        assert(changed && connections.status == STATIC_CONNECTION_CONFLICT);
        destroy_connections();
    }
}

/** A known receiver does not fill the written namespace's missing template parameters. */
static void it_rejects_open_qualified_namespaces_even_with_concrete_destination_evidence(void) {
    setup_connections("@!\nanchor\n//integer\nanchor\n /test/fixture/v1.0.0/src/other/value\n",
        "@=\nvalue\n.slot\n@<\nargument\n.slot\n@!\n");
    const StaticResultContext *actual;
    (void)prepare_operations(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    changed = true;
    assert(!static_connection_constant(&connections, connections.first_constant, actual, &changed));
    assert(changed && connections.status == STATIC_CONNECTION_UNRESOLVED && !connections.first_constant->equations_ready);
    destroy_connections();
}

/** Copied transfer identities, missing outputs and wrong namespace contexts fail exactly. */
static void it_refuses_foreign_constant_transfers_and_invalid_namespace_receivers(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup_connections("@!\nreceived\n /test/fixture/v1.0.0/src/other/value\n", "@=\nvalue\n//integer\n@!\n");
        const StaticResultContext *actual;
        (void)prepare_operations(&actual);
        bool changed;
        StaticResultOutcome outcome;
        assert(static_connection_assignments(&connections, actual, &changed, &outcome));
        StaticTypedConstant copied = *connections.first_constant;
        const StaticResultContext *supplier = constant_fixed_target_context();
        changed = true;
        assert(!static_connection_constant(&connections, kind == 0 ? &copied : connections.first_constant,
            kind == 1 ? actual : supplier, kind == 2 ? NULL : &changed));
        assert(changed && !connections.first_constant->equations_ready && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** A separately resolved shared initializer may already provide every equality;
 * validating its retained transfer still supplies one new finite proof fact.
 */
static void it_reports_new_transfer_proof_even_when_another_consumer_supplied_all_equations(void) {
    setup_connections("@=\nlocal\n//integer\n /test/fixture/v1.0.0/src/other/value\n@!\n", "@=\nvalue\n//integer\n@!\n");
    const StaticResultContext *actual;
    (void)prepare_operations(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_initializers(&connections, actual, &changed, &outcome));
    assert(outcome == STATIC_RESULT_PENDING && connections.constant_count == 1);
    const StaticTypedConstant *transfer = connections.first_constant;
    const StaticResultContext *supplier = constant_fixed_target_context();
    const AstNode *node = constant_named(caller_module, "local");
    const StaticConstantDeclaration *declaration;
    const StaticConstantInitializer *initializer;
    assert(static_constant_declaration(&constants, actual, node, &declaration));
    assert(static_constant_initializer(&constants, declaration, node->as.declaration.first->source, &initializer));
    assert(static_constant_initializer_resolve(&constants, initializer, supplier, &outcome));
    assert(outcome == STATIC_RESULT_READY && !transfer->equations_ready);
    size_t equations = solver.equation_count;
    assert(static_connection_constant(&connections, transfer, supplier, &changed));
    assert(transfer->equations_ready && solver.equation_count == equations);
    assert(changed);
    assert(static_connection_constant(&connections, transfer, supplier, &changed) && !changed);
    destroy_connections();
}

/** Every registration/selection/equation allocation refusal retains all owners. */
static void it_unwinds_every_qualified_transfer_allocation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_connections("@!\nreceived\n /test/fixture/v1.0.0/src/other/value\n", "@=\nvalue\n//integer\n@!\n");
            const StaticResultContext *actual;
            (void)prepare_operations(&actual);
            bool changed;
            StaticResultOutcome outcome;
            const StaticResultContext *supplier = NULL;
            if (phase) {
                assert(static_connection_assignments(&connections, actual, &changed, &outcome));
                supplier = constant_fixed_target_context();
            }
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            changed = false;
            outcome = STATIC_RESULT_PENDING;
            bool success = phase ? static_connection_constant(&connections, connections.first_constant, supplier, &changed) :
                static_connection_assignments(&connections, actual, &changed, &outcome);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && !changed && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_connections();
        }
    }
}

/** Exercise independent namespace closure, all source consumers and refusal ownership. */
static void run(void) {
    it_resolves_qualified_assignment_types_only_after_explicit_namespace_validation();
    it_connects_qualified_list_element_initializers_and_call_inputs();
    it_rejects_reference_and_scalar_list_storage_mismatches();
    it_rejects_open_qualified_namespaces_even_with_concrete_destination_evidence();
    it_refuses_foreign_constant_transfers_and_invalid_namespace_receivers();
    it_reports_new_transfer_proof_even_when_another_consumer_supplied_all_equations();
    it_unwinds_every_qualified_transfer_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
