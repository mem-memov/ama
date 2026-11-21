/** @file
 * @brief Check original assignment Type propagation, exact storage and refusal ownership.
 */
#include "../common.c"

/** Run original annotations before collecting own-body local equations. */
static const StaticTypedScope *prepare_assignments(const StaticResultContext **actual) {
    *actual = publication_context(caller_environment);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, *actual, &scope));
    bool changed;
    assert(static_connection_annotations(&connections, scope, &changed));
    return scope;
}

/** Query the original fixed module field, then demand its concrete Type. */
static void expect_type(const StaticTypedScope *scope, const char *name, const StaticTerm *expected) {
    const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, name));
    const StaticTypedField *field;
    assert(definition && static_connection_field(&connections, scope, definition, &field));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, field->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == expected);
}

/** All exact source storage categories propagate their declared module Type. */
static void it_propagates_owned_reference_list_index_acquire_continuation_and_self_types(void) {
    setup_connections("@!\nowned\n//integer\nindex\n//integer\nvalues[]\n//integer\nrefs&[]\n//integer\n"
        "copy\n owned\nborrowed&\n & owned\ncopy ref&\n borrowed&\ncopy list[]\n values[]\n"
        "copy refs&[]\n refs&[]\nselected\n values[index]\nselected ref&\n refs&[index]\n"
        "saved\n % borrowed&\nself&\n &\n", "@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_assignments(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(changed && outcome == STATIC_RESULT_READY);
    const char *names[] = {"copy", "borrowed", "copy ref", "copy list", "copy refs", "selected", "selected ref", "saved"};
    for (size_t i = 0; i < sizeof(names) / sizeof(*names); i++) { expect_type(scope, names[i], integer_type); }
    expect_type(scope, "self", caller_constructor);
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(!changed && outcome == STATIC_RESULT_READY && !actual->registry_use->candidate->local_complete);
    destroy_connections();
}

/** Qualified constants need independent namespace evidence rather than caller lookup. */
static void it_retains_deferred_sources_without_guessing_a_type_from_payloads(void) {
    setup_connections("@!\nfrom constant\n /test/fixture/v1.0.0/src/other/constant\n", "@=\nconstant\n//integer\n |1\n@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_assignments(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(!changed && outcome == STATIC_RESULT_PENDING);
    const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, "from constant"));
    const StaticTypedField *field;
    assert(static_connection_field(&connections, scope, definition, &field));
    const StaticTerm *value = NULL;
    StaticConstraintOutcome closed;
    assert(static_constraint_value(&solver, field->type, &value, &closed));
    assert(closed == STATIC_CONSTRAINT_PENDING && value == NULL);
    destroy_connections();
}

/** A literal initializer has no intrinsic Type and leaves the operand untouched. */
static void it_keeps_literal_initializer_capability_work_separate_from_type_sources(void) {
    setup_connections("@!\nknown\n//integer\n |123\n", "@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_assignments(&actual);
    const StaticRuntimeEvent *event = scope->runtime->first_event;
    while (event && event->kind != STATIC_RUNTIME_INITIALIZER) { event = event->next; }
    assert(event);
    StaticTypedOperand operand = {.type = caller_constructor, .reference = true, .list = true};
    StaticResultOutcome outcome;
    assert(static_typed_operand(&connections, scope, event, event->source, &operand, &outcome));
    assert(outcome == STATIC_RESULT_PENDING && operand.type == caller_constructor && operand.reference && operand.list);
    destroy_connections();
}

/** Copying an unknown source preserves underdetermination until an independent anchor exists. */
static void it_never_uses_unknown_to_unknown_copying_as_a_concrete_type_anchor(void) {
    setup_connections("@<\nsource\n.slot\n@!\nanchor\n//integer\ncopy\n source\n", "@!\n");
    const StaticResultContext *actual;
    (void)prepare_assignments(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    assert(changed && outcome == STATIC_RESULT_READY);
    assert(!static_constraint_finish(&solver) && solver.status == STATIC_CONSTRAINT_UNRESOLVED);
    destroy_connections();
}

/** Explicit receiver Type disagreement is a solver conflict, without rebinding. */
static void it_rejects_incompatible_explicit_type_equations(void) {
    setup_connections("@!\nsource\n//integer\ntarget\n//text\ntarget\n source\n", "@!\n");
    const StaticResultContext *actual;
    (void)prepare_assignments(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_assignments(&connections, actual, &changed, &outcome));
    StaticConstraintOutcome closed;
    assert(!static_constraint_solve(&solver, &closed) && solver.status == STATIC_CONSTRAINT_CONFLICT);
    destroy_connections();
}

/** Exact suffixes and continuation direct identity are checked before capabilities. */
static void it_rejects_storage_mismatches_and_direct_continuation_self_copy(void) {
    const char *sources[] = {
        "@!\nsource\n//integer\ntarget&\n source\n",
        "@!\nsource[]\n//integer\ntarget\n source[]\n",
        "@!\nsource\n//integer\nsource\n % source\n"
    };
    for (size_t kind = 0; kind < sizeof(sources) / sizeof(*sources); kind++) {
        setup_connections(sources[kind], "@!\n");
        const StaticResultContext *actual;
        (void)prepare_assignments(&actual);
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_assignments(&connections, actual, &changed, &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_CONFLICT);
        destroy_connections();
    }
}

/** Foreign context pointers and missing outputs cannot certify own-body work. */
static void it_refuses_foreign_contexts_and_absent_assignment_outputs(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup_connections("@!\nanchor\n//integer\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        StaticResultContext copied = *actual;
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_assignments(&connections, kind == 0 ? &copied : actual,
            kind == 1 ? NULL : &changed, kind == 2 ? NULL : &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** Measured actual-scope and equality allocations all retain owned partial work. */
static void it_unwinds_every_assignment_equation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_connections("@!\nsource\n//integer\ntarget\n source\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool changed = false;
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        bool success = static_connection_assignments(&connections, actual, &changed, &outcome);
        if (!failure) { assert(success && changed); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !changed && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_connections();
    }
}

/** Exercise all local source categories, deferred evidence, conflicts and failures. */
static void run(void) {
    it_propagates_owned_reference_list_index_acquire_continuation_and_self_types();
    it_retains_deferred_sources_without_guessing_a_type_from_payloads();
    it_keeps_literal_initializer_capability_work_separate_from_type_sources();
    it_never_uses_unknown_to_unknown_copying_as_a_concrete_type_anchor();
    it_rejects_incompatible_explicit_type_equations();
    it_rejects_storage_mismatches_and_direct_continuation_self_copy();
    it_refuses_foreign_contexts_and_absent_assignment_outputs();
    it_unwinds_every_assignment_equation_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
