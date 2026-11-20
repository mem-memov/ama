/** @file
 * @brief Check ordinary interface metadata independently of actual callee expansion.
 */
#include "common.c"

/** A known generic origin exposes a frozen interface while captures stay unknown. */
static void it_prepares_symbolic_callee_interfaces_without_creating_callee_contexts(void) {
    setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < anchor\n > fresh\n",
        "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    const AstNode *node = operation_at(scope, 0);
    size_t contexts = results.context_count;
    const StaticTypedOperation *operation;
    StaticResultOutcome outcome;
    assert(static_connection_operation(&connections, actual, node, &operation, &outcome));
    assert(outcome == STATIC_RESULT_READY && operation->target->environment->definition == target_module);
    assert(!operation->target->owner->ground && results.context_count == contexts && connections.operation_count == 1);
    assert(operation->bindings->input_count == 1 && operation->bindings->output_count == 1);
    assert(operation->bindings->first->slot->runtime_declaration != NULL);
    size_t before = allocation_count;
    const StaticTypedOperation *same;
    assert(static_connection_operation(&connections, actual, node, &same, &outcome));
    assert(same == operation && allocation_count == before && !actual->registry_use->candidate->local_complete);
    destroy_connections();
}

/** Expected request labels never supply an origin; a later declared Type can. */
static void it_retains_unknown_target_origins_until_an_independent_type_anchor_arrives(void) {
    setup_connections("@<\nworker\n.slot\n@!\nanchor\n//integer\nworker !\n", "@!\n");
    const StaticResultContext *actual;
    const StaticTypedScope *scope = prepare_operations(&actual);
    const AstNode *node = operation_at(scope, 0);
    const StaticTypedOperation *operation = NULL;
    StaticResultOutcome outcome;
    assert(static_connection_operation(&connections, actual, node, &operation, &outcome));
    assert(outcome == STATIC_RESULT_PENDING && operation == NULL && connections.operation_count == 0);
    const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, "worker"));
    const StaticTypedField *field;
    assert(static_connection_field(&connections, scope, definition, &field));
    assert(static_constraint_equate(&solver, field->type, target_constructor, definition->site));
    assert(static_connection_operation(&connections, actual, node, &operation, &outcome));
    assert(outcome == STATIC_RESULT_READY && operation->target->environment->definition == target_module);
    destroy_connections();
}

/** Selected targets resolve only through a prepared parent's public result sequence. */
static void it_resolves_selected_targets_without_private_or_caller_name_fallback(void) {
    for (size_t private = 0; private < 2; private++) {
        setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n method& !\n",
            private ? "@!\nmethod&\n//integer\n" : "@>\nmethod&\n//integer\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const StaticTypedOperation *operation = NULL;
        StaticResultOutcome outcome;
        const AstNode *parent = operation_at(scope, 0), *child = operation_at(scope, 1);
        assert(static_connection_operation(&connections, actual, child, &operation, &outcome));
        assert(outcome == STATIC_RESULT_PENDING && operation == NULL);
        assert(static_connection_operation(&connections, actual, parent, &operation, &outcome));
        bool success = static_connection_operation(&connections, actual, child, &operation, &outcome);
        if (private) { assert(!success && connections.status == STATIC_CONNECTION_CONFLICT); }
        else { assert(success && outcome == STATIC_RESULT_READY && operation->target->owner == integer_type); }
        destroy_connections();
    }
}

/** Copied nodes and absent outputs do not belong to the actual caller catalog. */
static void it_refuses_foreign_operation_syntax_and_absent_output_receivers(void) {
    for (size_t kind = 0; kind < 3; kind++) {
        setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n", "@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const AstNode *node = operation_at(scope, 0);
        AstNode copied = *node;
        const StaticTypedOperation *operation = NULL;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_connection_operation(&connections, actual, kind == 0 ? &copied : node,
            kind == 1 ? NULL : &operation, kind == 2 ? NULL : &outcome));
        assert(operation == NULL && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** Every observed metadata preparation allocation can refuse without publishing a call. */
static void it_unwinds_every_operation_interface_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_connections("@!\nanchor\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < anchor\n > fresh\n",
            "@<\nargument\n.slot\n@>\nreturned\n.slot\n@!\n");
        const StaticResultContext *actual;
        const StaticTypedScope *scope = prepare_operations(&actual);
        const AstNode *node = operation_at(scope, 0);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        const StaticTypedOperation *operation = NULL;
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        bool success = static_connection_operation(&connections, actual, node, &operation, &outcome);
        if (!failure) { assert(success && operation); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !operation && outcome == STATIC_RESULT_PENDING && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_connections();
    }
}

/** Exercise proven/unknown origins, public selection, exact identity and failures. */
static void run(void) {
    it_prepares_symbolic_callee_interfaces_without_creating_callee_contexts();
    it_retains_unknown_target_origins_until_an_independent_type_anchor_arrives();
    it_resolves_selected_targets_without_private_or_caller_name_fallback();
    it_refuses_foreign_operation_syntax_and_absent_output_receivers();
    it_unwinds_every_operation_interface_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
