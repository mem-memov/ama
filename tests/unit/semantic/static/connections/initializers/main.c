/** @file
 * @brief Check initializer/default Type inference independently of value execution.
 */
#include "../common.c"

/** Retain the caller's real incoming context and submit finite annotation facts. */
static const StaticTypedScope *prepare_initializers(const StaticResultContext **actual) {
    *actual = publication_context(caller_environment);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, *actual, &scope));
    bool changed;
    assert(static_connection_annotations(&connections, scope, &changed));
    return scope;
}

/** A concrete source constrains input, output and permanent-field placeholder Types. */
static void it_infers_default_result_and_list_element_types_from_declared_sources(void) {
    setup_connections("@=\nanchor\n//integer\n |1\n@<\nargument\n.slot\n anchor\n@>\nresult\n.slot\n argument\n@!\nvalues[]\n.slot\n anchor\n argument\n", "@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_initializers(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_initializers(&connections, actual, &changed, &outcome));
    assert(changed && outcome == STATIC_RESULT_PENDING); /* The anchor literal still needs capability checking. */
    for (const StaticTypedField *field = scope->first_field; field; field = field->next) {
        const StaticTerm *value;
        StaticConstraintOutcome closed;
        assert(static_constraint_value(&solver, field->type, &value, &closed));
        assert(closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    }
    assert(static_connection_initializers(&connections, actual, &changed, &outcome) && !changed);
    assert(!actual->registry_use->candidate || !actual->registry_use->candidate->local_complete);
    destroy_connections();
}

/** One owned/ref list initializer receives exactly one scalar of matching ownership. */
static void it_accepts_owned_and_reference_element_sources_without_splicing_lists(void) {
    setup_connections("@!\nanchor\n//integer\nvalues[]\n//integer\n anchor\nrefs&[]\n//integer\n & anchor\n", "@!\n");
    const StaticResultContext *actual;
    (void)prepare_initializers(&actual);
    bool changed;
    StaticResultOutcome outcome;
    assert(static_connection_initializers(&connections, actual, &changed, &outcome));
    assert(outcome == STATIC_RESULT_READY);
    destroy_connections();
}

/** A whole list cannot become one element and a reference cannot become owned. */
static void it_rejects_list_splicing_implicit_dereference_and_reference_literals(void) {
    const char *sources[] = {
        "@!\nsource[]\n//integer\ntarget[]\n//integer\n source[]\n",
        "@!\nsource&\n//integer\ntarget\n//integer\n source&\n",
        "@!\ntarget&\n//integer\n |1\n"
    };
    for (size_t kind = 0; kind < sizeof(sources) / sizeof(*sources); kind++) {
        setup_connections(sources[kind], "@!\n");
        const StaticResultContext *actual;
        (void)prepare_initializers(&actual);
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_initializers(&connections, actual, &changed, &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_CONFLICT);
        destroy_connections();
    }
}

/** Foreign context pointers and absent result receivers preserve caller outputs. */
static void it_refuses_foreign_contexts_and_missing_initializer_outputs(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup_connections("@!\nanchor\n//integer\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        StaticResultContext copied = *actual;
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_initializers(&connections, kind == 0 ? &copied : actual,
            kind == 1 ? NULL : &changed, kind == 2 ? NULL : &outcome));
        assert(changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** Every observed context/field/equation allocation can refuse with owned partial work. */
static void it_unwinds_every_initializer_equation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_connections("@!\nanchor\n//integer\ncopy\n//integer\n anchor\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool changed = false;
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        bool success = static_connection_initializers(&connections, actual, &changed, &outcome);
        if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !changed && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_connections();
    }
}

/** Exercise typed defaults, scalar list elements, exact storage and owned failures. */
static void run(void) {
    it_infers_default_result_and_list_element_types_from_declared_sources();
    it_accepts_owned_and_reference_element_sources_without_splicing_lists();
    it_rejects_list_splicing_implicit_dereference_and_reference_literals();
    it_refuses_foreign_contexts_and_missing_initializer_outputs();
    it_unwinds_every_initializer_equation_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
