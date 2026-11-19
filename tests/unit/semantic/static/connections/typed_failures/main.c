/** @file
 * @brief Verify typed scope/context/field identities and all association/congruence allocation refusals.
 */
#include "../typed/common.c"

/** Generic environments have no invented actual root. Supplied nominal metadata
 * does not make unrelated copied environments/contexts valid checked identities.
 */
static void it_refuses_unrooted_generic_scopes_copied_environments_and_context_receivers(void) {
    for (size_t kind = 0; kind < 7; kind++) {
        setup_typed();
        const StaticTypedScope *scope = NULL;
        const StaticResultContext *actual = NULL;
        if (kind >= 3) { actual = typed_actual(); }
        StaticTermEnvironment copied = *caller_environment;
        if (kind < 3) {
            assert(!static_connection_scope(&connections, kind == 1 ? &copied : caller_environment, kind == 2 ? NULL : &scope));
        } else {
            StaticResultContext copied_context = *actual;
            if (kind == 6) { connections.context_count = SIZE_MAX; }
            assert(!static_connection_context(&connections, kind == 3 ? &copied_context : kind == 4 ? NULL : actual,
                kind == 5 ? NULL : &scope));
        }
        assert(scope == NULL && connections.status == (kind == 6 ? STATIC_CONNECTION_OUT_OF_MEMORY : STATIC_CONNECTION_INVALID));
        assert(connections.context_count == (kind == 6 ? SIZE_MAX : 0));
        if (kind == 0) { assert(guards.path_count == 0 && results.context_count == 0 && connections.scope_count == 0); }
        destroy_connections();
    }
}

/** Exact source binding membership selects typed fields; same payloads cannot
 * substitute for the original scope/declaration or omit public output receivers.
 */
static void it_refuses_copied_typed_fields_scopes_missing_outputs_and_scope_counter_wrap(void) {
    for (size_t kind = 0; kind < 6; kind++) {
        setup_typed();
        const StaticResultContext *actual = typed_actual();
        const StaticTypedScope *scope = NULL;
        if (kind == 5) {
            connections.scope_count = SIZE_MAX;
            assert(!static_connection_scope(&connections, caller_environment, &scope));
            assert(scope == NULL && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY);
        } else {
            assert(static_connection_context(&connections, actual, &scope));
            const StaticTypedField *field = typed_field(scope, "fresh"), *output = NULL;
            StaticTypedScope copied = *scope;
            StaticRuntimeField copied_field = *field->definition;
            if (kind == 4) { assert(!static_connection_merge_fields(&connections, NULL)); }
            else {
                assert(!static_connection_field(&connections, kind == 0 ? &copied : scope,
                    kind == 1 ? &copied_field : kind == 2 ? NULL : field->definition, kind == 3 ? NULL : &output));
            }
            assert(output == NULL && connections.status == STATIC_CONNECTION_INVALID);
        }
        assert(diagnostic_valid(&connections.error));
        destroy_connections();
    }
}

/** Every metadata-scope/actual-context allocation may refuse; published source
 * metadata is not executable admission and all failed partial fields stay owned.
 */
static void it_unwinds_every_typed_scope_and_actual_context_preparation_allocation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t cut = 0; cut <= total; cut++) {
            setup_typed();
            const StaticResultContext *actual = phase ? typed_actual() : NULL;
            const StaticTermEnvironment *environment = phase ? caller_environment : typed_environment(integer_type);
            const StaticTypedScope *scope = NULL;
            size_t before = allocation_count;
            if (cut) { failure_at = before + cut; }
            bool success = phase ? static_connection_context(&connections, actual, &scope) : static_connection_scope(&connections, environment, &scope);
            if (!cut) { assert(success && scope); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && scope == NULL && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY);
                assert(connections.context_count == 0 && diagnostic_valid(&connections.error));
            }
            failure_at = 0;
            destroy_connections();
        }
    }
}

/** Scratch and persistent equality refusal cannot publish a changed result;
 * already prepared scopes/context/terms remain safely destructible.
 */
static void it_unwinds_every_corresponding_field_congruence_join_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_typed();
        const StaticTypedScope *root, *metadata;
        assert(static_connection_context(&connections, typed_actual(), &root));
        assert(static_connection_scope(&connections, typed_environment(integer_type), &metadata));
        const StaticTerm *slot = application_symbol("...slot");
        assert(static_constraint_equate(&solver, slot, integer_type, slot->site));
        StaticConstraintOutcome outcome;
        assert(static_constraint_solve(&solver, &outcome));
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool changed = false;
        bool success = static_connection_merge_fields(&connections, &changed);
        if (!cut) { assert(success && changed); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !changed && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY && diagnostic_valid(&connections.error)); }
        failure_at = 0;
        destroy_connections();
    }
}

/** Static graph modules have no ordinary fields or owner Type, even when a
 * real compilation root/context is registered for their producer checks.
 */
static void it_refuses_static_modules_as_runtime_or_typed_instance_scopes(void) {
    for (size_t operation = 0; operation < 3; operation++) {
        setup_connections("@!\nanchor&\n..integer\n", "@...\n...out\n..integer\n");
        const StaticTermEnvironment *environment;
        bool success = static_term_environment(&terms, target_module, NULL, &environment);
        assert(success);
        const StaticResultContext *actual = operation == 2 ? publication_context(environment) : NULL;
        const StaticRuntimeModule *runtime = NULL;
        const StaticTypedScope *scope = NULL;
        size_t before = allocation_count;
        if (operation == 0) { success = static_connection_runtime(&connections, target_module, &runtime); }
        else if (operation == 1) { success = static_connection_scope(&connections, environment, &scope); }
        else { success = static_connection_context(&connections, actual, &scope); }
        assert(!success && runtime == NULL && scope == NULL && connections.status == STATIC_CONNECTION_INVALID);
        assert(allocation_count == before && connections.runtime_count == 0 && connections.scope_count == 0);
        assert(constants.status == STATIC_CONSTANT_OK && publications.status == STATIC_PUBLICATION_OK);
        assert(solver.status == STATIC_CONSTRAINT_OK);
        destroy_connections();
    }
}

/** Run independent malformed, counter and preparation/query refusal partitions. */
static void run(void) {
    it_refuses_static_modules_as_runtime_or_typed_instance_scopes();
    it_refuses_unrooted_generic_scopes_copied_environments_and_context_receivers();
    it_refuses_copied_typed_fields_scopes_missing_outputs_and_scope_counter_wrap();
    it_unwinds_every_typed_scope_and_actual_context_preparation_allocation_refusal();
    it_unwinds_every_corresponding_field_congruence_join_allocation_refusal();
}
/** Return only after every assertion and counted teardown. */
int main(void) { run(); return 0; }
