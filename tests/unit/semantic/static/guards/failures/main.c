/** @file
 * @brief Refuse each public guard allocation and validate checked publication counters.
 */
#include "operations.c"

/** Every actual allocator request is refused once in a fresh identical owner;
 * even mid-refresh publication retains destructible owned metadata only.
 */
static void it_unwinds_every_public_root_enter_refresh_and_definition_allocation_refusal(void) {
    for (size_t mode = 0; mode < 8; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            GuardOperation operation = prepare(mode);
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = execute(mode, operation);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && guards.status == STATIC_GUARD_OUT_OF_MEMORY); }
            failure_at = 0;
            assert(inventory.valid && caller_module->parameter_count == 2);
            destroy_guards();
        }
    }
}

/** Build two abstract already-normalized creation endpoints for scratch-only
 * kernel tests. The source trace is retained metadata, not source evaluation.
 */
static void creation_graph(bool cyclic, StaticCreationPath *trace) {
    StaticTermCapture captures[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTerm *value;
    assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &value));
    const StaticTerm *closed;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, value, &closed, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    StaticCreationVertex *source, *target;
    assert(static_guard_vertex(&guards, closed, &source));
    if (cyclic) { target = source; }
    else {
        captures[0].formal = application_formal(target_module, 0);
        captures[1].formal = application_formal(target_module, 1);
        assert(static_term_nominal(&terms, target_module, captures, 2, (SourceSpan){0}, &value));
        assert(static_constraint_value(&solver, value, &closed, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
        assert(static_guard_vertex(&guards, closed, &target));
    }
    StaticCreationEdge *edge = static_guard_allocate(&guards, sizeof(*edge));
    assert(edge);
    *edge = (StaticCreationEdge){.target = target, .path = trace};
    source->first = source->last = edge;
    guards.edge_count = 1;
}

/** Both success and semantic-cycle kernels allocate scratch before graph search;
 * refusal must clean it without allocating to explain or retain an error.
 */
static void it_unwinds_every_successful_and_rejected_graph_search_allocation_refusal(void) {
    for (size_t mode = 0; mode < 4; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_guards(caller_source, target_source);
            StaticCreationPath trace = {.site = named("...a")->site};
            bool cyclic = mode % 2 != 0;
            if (mode < 2) { creation_graph(cyclic, &trace); }
            else {
                const StaticCreationPath *root;
                StaticDefinitionNode *a, *b;
                assert(static_creation_root(&guards, caller_environment, &root));
                assert(static_guard_definition(&guards, root, named("...a"), &a));
                assert(static_guard_definition(&guards, root, named("...b"), &b));
                StaticDefinitionEdge *edge = static_guard_allocate(&guards, sizeof(*edge));
                assert(edge);
                *edge = (StaticDefinitionEdge){.target = cyclic ? a : b, .site = named("...a")->site};
                a->first = a->last = edge;
                guards.definition_edge_count = 1;
            }
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = mode < 2 ? static_guard_creation_graph(&guards) : static_guard_definition_graph(&guards);
            if (!failure) {
                assert(success == !cyclic);
                assert(guards.status == (cyclic ? STATIC_GUARD_CONFLICT : STATIC_GUARD_OK));
                total = allocation_count - before;
                assert(total > 0);
            } else { assert(!success && guards.status == STATIC_GUARD_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_guards();
        }
    }
}

/** Dense counters/products fail before wrapping into indices or allocations.
 * Tests mutate only explicit limit fields, then destroy the failed owner.
 */
static void it_guards_path_vertex_definition_and_search_counter_limits(void) {
    for (size_t mode = 0; mode < 8; mode++) {
        setup_guards(caller_source, target_source);
        const StaticCreationPath *root;
        const StaticGuardOwner *owner;
        assert(static_guard_owner(&guards, caller_environment, &owner));
        if (mode == 0) {
            guards.path_count = SIZE_MAX;
            size_t before = allocation_count;
            const StaticCreationPath *output = NULL;
            assert(!static_creation_root(&guards, caller_environment, &output));
            assert(output == NULL && allocation_count == before);
        } else if (mode == 1) {
            const StaticTerm *closed;
            StaticConstraintOutcome outcome;
            assert(static_constraint_value(&solver, integer_type, &closed, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
            guards.vertex_count = SIZE_MAX;
            StaticCreationVertex *output = NULL;
            size_t before = allocation_count;
            assert(!static_guard_vertex(&guards, closed, &output));
            assert(output == NULL && allocation_count == before);
        } else {
            assert(static_creation_root(&guards, caller_environment, &root));
            if (mode == 2) {
                guards.definition_count = SIZE_MAX;
                const StaticDefinitionPath *output = NULL;
                assert(!static_definition_enter(&guards, NULL, root, named("...a"), (SourceSpan){0}, &output));
                assert(output == NULL);
            } else if (mode == 3) {
                StaticCreationPath trace = {.site = named("...a")->site};
                creation_graph(false, &trace);
                guards.vertex_count = SIZE_MAX;
                size_t before = allocation_count;
                assert(!static_guard_creation_graph(&guards) && allocation_count == before);
            } else if (mode == 4) {
                StaticDefinitionNode *node;
                assert(static_guard_definition(&guards, root, named("...a"), &node));
                guards.definition_count = SIZE_MAX;
                guards.definition_edge_count = 1;
                size_t before = allocation_count;
                assert(!static_guard_definition_graph(&guards) && allocation_count == before);
            } else if (mode == 5) {
                const StaticApplicationRecord *record = application(caller_environment);
                const StaticCreationPath *child;
                StaticCreationOutcome outcome;
                assert(static_creation_enter(&guards, root, record, &child, &outcome));
                anchor(root->owner->value);
                anchor(child->owner->value);
                guards.edge_count = SIZE_MAX;
                StaticGuardClosure closure = STATIC_GUARD_PENDING;
                assert(!static_guard_refresh(&guards, &closure));
                assert(closure == STATIC_GUARD_PENDING && child->edge == NULL);
            } else {
                const StaticDefinitionPath *first, *output = NULL;
                assert(static_definition_enter(&guards, NULL, root, named("...a"), (SourceSpan){0}, &first));
                if (mode == 6) { guards.definition_edge_count = SIZE_MAX; }
                else { guards.definition_path_count = SIZE_MAX; }
                SourceSpan site = named("...a")->declaration->as.static_result.expression->span;
                assert(!static_definition_enter(&guards, mode == 6 ? first : NULL, root, named("...b"), site, &output));
                assert(output == NULL);
            }
        }
        assert(guards.status == STATIC_GUARD_OUT_OF_MEMORY);
        destroy_guards();
    }
}

/** Missing outputs or copied application handles never expose an expansion;
 * a borrowed application error remains the same first diagnostic after propagation.
 */
static void it_refuses_missing_receivers_and_preserves_borrowed_first_errors(void) {
    for (size_t partition = 0; partition < 6; partition++) {
        GuardOperation operation = prepare(2);
        const StaticCreationPath *output = NULL;
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        StaticGuardClosure closure = STATIC_GUARD_PENDING;
        StaticApplicationRecord copied = *operation.application;
        bool success;
        if (partition == 0) { success = static_creation_root(&guards, caller_environment, NULL); }
        else if (partition == 1) { success = static_guard_refresh(&guards, NULL); }
        else {
            if (partition == 5) {
                StaticApplicationOutcome checked;
                assert(!static_application_check(&applications, &copied, &checked));
            }
            success = static_creation_enter(&guards, operation.root, partition == 2 ? &copied : operation.application,
                partition == 3 ? NULL : &output, partition == 4 ? NULL : &outcome);
        }
        assert(!success && output == NULL && outcome == STATIC_CREATION_PENDING && guards.status == STATIC_GUARD_INVALID);
        assert(guards.path_count == 1);
        const char *message = guards.error.message;
        if (partition == 5) { assert(message == applications.error.message); }
        assert(!static_guard_refresh(&guards, &closure) && guards.error.message == message);
        destroy_guards();
    }
}

/** Execute public cutpoints, both graph scratch outcomes and checked limits. */
static void run(void) {
    it_unwinds_every_public_root_enter_refresh_and_definition_allocation_refusal();
    it_unwinds_every_successful_and_rejected_graph_search_allocation_refusal();
    it_guards_path_vertex_definition_and_search_counter_limits();
    it_refuses_missing_receivers_and_preserves_borrowed_first_errors();
}
/** Return success after every refused preparation is fully destroyed. */
int main(void) { run(); return 0; }
