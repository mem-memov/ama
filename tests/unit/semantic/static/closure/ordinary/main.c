/** @file
 * @brief Verify real ordinary equation-pass composition without treating metadata as admission.
 */
#include "../../connections/common.c"
#include "semantic/static/closure/ordinary/pass.h"

/** Resolve a test expectation through the already fixed original field identity. */
static const StaticTypedField *field_named(const StaticTypedScope *scope, const char *name) {
    const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, name));
    assert(definition);
    const StaticTypedField *field;
    assert(static_connection_field(&connections, scope, definition, &field));
    return field;
}

/** Independent known assignment evidence closes the original interface hole. */
static void it_closes_fields_from_actual_body_equations_without_admission(void) {
    setup_connections("@<\ninput\n...slot\n@!\nknown\n..integer\ninput\n known\n", "@!\n");
    const StaticResultContext *actual = publication_context(caller_environment);
    StaticOrdinaryPass report;
    assert(static_closure_ordinary_pass(&connections, actual, &report));
    assert(report.changed && report.fields_closed && report.transfers_ready);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, field_named(scope, "input")->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(actual->registry_use->candidate == NULL || !actual->registry_use->candidate->local_complete);
    assert(static_closure_ordinary_pass(&connections, actual, &report));
    assert(!report.changed && report.fields_closed && report.transfers_ready);
    destroy_connections();
}

/** A literal supplies capability work, never a guessed Type anchor. */
static void it_separates_literal_obligations_from_field_type_closure(void) {
    for (size_t known = 0; known < 2; known++) {
        setup_connections(known ? "@!\nvalue\n..integer\n |7\n" : "@<\nvalue\n...slot\n |7\n@!\nanchor&\n..integer\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        StaticOrdinaryPass report;
        assert(static_closure_ordinary_pass(&connections, actual, &report));
        assert(report.fields_closed == (known != 0) && !report.transfers_ready);
        assert(static_closure_ordinary_pass(&connections, actual, &report));
        assert(!report.changed && !report.transfers_ready);
        destroy_connections();
    }
}

/** A Pending first call must not suppress a later independently known result. */
static void it_continues_past_unknown_calls_and_retries_with_new_evidence(void) {
    setup_connections("@<\nunknown\n...slot\n@!\nunknown !\nworker\n.other\nworker !\n > fresh\n", "@>\nvalue\n..integer\n@!\n");
    const StaticResultContext *actual = publication_context(caller_environment);
    StaticOrdinaryPass report;
    assert(static_closure_ordinary_pass(&connections, actual, &report));
    assert(!report.fields_closed && !report.transfers_ready);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, field_named(scope, "fresh")->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(static_constraint_equate(&solver, field_named(scope, "unknown")->type, integer_type, (SourceSpan){0}));
    assert(static_closure_ordinary_pass(&connections, actual, &report));
    assert(report.fields_closed && report.transfers_ready);
    /* Interface preparation cannot invent an actual context for worker. */
    for (const StaticTypedContext *context = connections.first_context; context; context = context->next) {
        assert(context->actual == actual);
    }
    destroy_connections();
}

/** Empty traversal bodies still constrain their original local reference. */
static void it_composes_foreach_and_local_assignments(void) {
    setup_connections("@!\nvalues[]\n..integer\nvalues[item&]\n{\n borrowed&\n  item&\n}\n", "@!\n");
    const StaticResultContext *actual = publication_context(caller_environment);
    StaticOrdinaryPass report;
    assert(static_closure_ordinary_pass(&connections, actual, &report));
    assert(report.fields_closed && report.transfers_ready);
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, field_named(scope, "borrowed")->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    destroy_connections();
}

/** Conflicting child evidence and invalid provenance preserve report receivers. */
static void it_preserves_outputs_on_conflict_or_invalid_context(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup_connections("@!\nfirst\n..integer\nsecond\n..text\nfirst\n second\n", "@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        StaticResultContext copied = *actual;
        StaticOrdinaryPass report = {.fields_closed = true, .transfers_ready = true};
        bool success = static_closure_ordinary_pass(&connections, kind == 1 ? &copied : actual, kind == 2 ? NULL : &report);
        assert(!success && !report.changed && report.fields_closed && report.transfers_ready);
        assert(connections.status == (kind ? STATIC_CONNECTION_INVALID : STATIC_CONNECTION_CONFLICT));
        destroy_connections();
    }
}

/** Fail every allocation in the composed pass, including solver scratch and
 * actual annotation validation, and unwind every retained partial child owner. */
static void it_unwinds_every_composed_pass_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_connections("@<\ninput\n...slot\n@!\nknown\n..integer\ninput\n known\nworker\n.other\nworker !\n > fresh\n",
            "@>\nvalue\n..integer\n@!\n");
        const StaticResultContext *actual = publication_context(caller_environment);
        StaticOrdinaryPass report = {.transfers_ready = true};
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool success = static_closure_ordinary_pass(&connections, actual, &report);
        if (!cut) {
            assert(success && report.fields_closed && report.transfers_ready);
            total = allocation_count - before;
            assert(total > 0);
        } else {
            assert(!success && !report.changed && !report.fields_closed && report.transfers_ready);
            assert(connections.status == STATIC_CONNECTION_OUT_OF_MEMORY && diagnostic_valid(&connections.error));
        }
        failure_at = 0;
        destroy_connections();
    }
}

/** Explicit scenario dispatch retains the project's independent leaf contract. */
void run(void) {
    it_closes_fields_from_actual_body_equations_without_admission();
    it_separates_literal_obligations_from_field_type_closure();
    it_continues_past_unknown_calls_and_retries_with_new_evidence();
    it_composes_foreach_and_local_assignments();
    it_preserves_outputs_on_conflict_or_invalid_context();
    it_unwinds_every_composed_pass_allocation_refusal();
}

/** Exercise fixture teardown and all assertions through the actual leaf entry. */
int main(void) { run(); return 0; }
