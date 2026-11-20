/** @file
 * @brief Verify complete Type inference across actual required bodies without claiming executable admission.
 */
#include "../../connections/common.c"
#include "semantic/static/closure/plan.h"
#include <stdio.h>

/** Worklist metadata expires before every borrowed equation/source owner. */
static StaticClosurePlan closure;

/** Retain exactly one explicitly selected caller root before refinement begins. */
static const StaticResultContext *setup_scheduler(const char *caller, const char *target) {
    setup_connections(caller, target);
    static_closure_init(&closure, &connections, sources.allocator);
    return publication_context(caller_environment);
}

/** Even rejected partial fixed points must release all counted ownership. */
static void destroy_scheduler(void) {
    static_closure_destroy(&closure);
    destroy_connections();
}

/** Inspect source-owned field Types independently of scheduler readiness flags. */
static const StaticTerm *field_type(const StaticResultContext *actual, const char *name) {
    const StaticTypedScope *scope;
    assert(static_connection_context(&connections, actual, &scope));
    const StaticRuntimeField *definition = static_runtime_field(scope->runtime, static_name_site(project.entry, name));
    assert(definition);
    const StaticTypedField *field;
    assert(static_connection_field(&connections, scope, definition, &field));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, field->type, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE);
    return value;
}

/** Ordinary inputs can specialize an actual callee and its independently saved result. */
static void it_closes_generic_calls_and_requires_their_actual_definitions(void) {
    const StaticResultContext *root = setup_scheduler(
        "@!\nknown\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < known\n > fresh\n",
        "@<\ninput\n.slot\n@>\noutput\n.slot\n@!\noutput\n input\n");
    assert(static_closure_resolve(&closure) && closure.closed);
    assert(field_type(root, "fresh") == integer_type);
    bool found = false;
    for (const StaticClosureEntry *entry = closure.first; entry; entry = entry->next) {
        if (entry->scope->environment->definition == target_module) { found = true; }
        if (entry->context->registry_use->candidate) {
            assert(!entry->context->registry_use->candidate->local_complete);
        }
    }
    assert(found && static_closure_resolve(&closure) && closure.closed);
    destroy_scheduler();
}

/** Callee body equations may supply output Type evidence before its key closes. */
static void it_infers_callee_output_types_from_required_body_equations(void) {
    const StaticResultContext *root = setup_scheduler(
        "@!\nanchor&\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n > fresh\n",
        "@>\noutput\n.slot\n@!\nknown\n//integer\noutput\n known\n");
    assert(static_closure_resolve(&closure) && closure.closed);
    assert(field_type(root, "fresh") == integer_type);
    destroy_scheduler();
}

/** Constant namespaces close independently before copied values infer receiver Types. */
static void it_closes_direct_and_saved_constant_receivers_without_value_execution(void) {
    for (size_t saved = 0; saved < 2; saved++) {
        const StaticResultContext *root = setup_scheduler(saved ?
            "@!\n/test/fixture/v1.0.0/src/other\n value >> received\n" : "@!\nreceived\n /test/fixture/v1.0.0/src/other/value\n",
            "@=\nvalue\n//integer\n |7\n@!\n");
        assert(static_closure_resolve(&closure) && closure.closed);
        assert(field_type(root, "received") == integer_type);
        assert(result_registry.dependency_count > 0);
        if (saved) { assert(constants.selection_count == 1); }
        else { assert(connections.first_constant->equations_ready); }
        destroy_scheduler();
    }
}

/** Empty references require their Type's actual body even without any call. */
static void it_rejects_bad_required_definitions_behind_empty_references(void) {
    (void)setup_scheduler("@!\nanchor&\n//integer\nunused&\n/test/fixture/v1.0.0/src/other\n",
        "@!\nfirst\n//integer\nsecond\n//text\nfirst\n second\n");
    assert(!static_closure_resolve(&closure) && !closure.closed);
    assert(closure.status == STATIC_CLOSURE_CONFLICT && diagnostic_valid(&closure.error));
    destroy_scheduler();
}

/** A literal never selects the missing interface Type; final closure refuses it. */
static void it_rejects_unanchored_required_types_instead_of_guessing(void) {
    (void)setup_scheduler("@<\ninput\n.slot\n |7\n@!\nanchor&\n//integer\n", "@!\n");
    assert(!static_closure_resolve(&closure) && !closure.closed);
    assert(closure.status == STATIC_CLOSURE_UNRESOLVED && diagnostic_valid(&closure.error));
    destroy_scheduler();
}

/** Exact-key nominal reference recursion must close with one body per key. */
static void it_closes_same_key_reference_cycles_without_repeated_body_expansion(void) {
    (void)setup_scheduler("@<\ninput\n.slot\n@!\nanchor\n//integer\ninput\n anchor\nself&\n/test/fixture/v1.0.0/src/main\n &\n", "@!\n");
    assert(static_closure_resolve(&closure) && closure.closed);
    size_t own_bodies = 0;
    for (const StaticClosureEntry *entry = closure.first; entry; entry = entry->next) {
        if (entry->representative == entry && entry->scope->environment->definition == caller_module) { own_bodies++; }
    }
    assert(own_bodies == 1);
    size_t entries = closure.count, paths = guards.path_count;
    assert(static_closure_resolve(&closure) && closure.count == entries && guards.path_count == paths);
    destroy_scheduler();
}

/** Distinct independent selected keys of one source remain separate in either order. */
static void it_preserves_independent_root_acceptance_in_both_discovery_orders(void) {
    for (size_t reverse = 0; reverse < 2; reverse++) {
        setup_connections("@<\ninput\n.slot\n@!\ninteger anchor&\n//integer\ntext anchor&\n//text\n", "@!\n");
        static_closure_init(&closure, &connections, sources.allocator);
        const StaticModule *text = static_module_find(&inventory, project_find(&project, "//text"));
        const StaticTerm *text_type;
        assert(static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &text_type));
        const StaticTerm *values[] = {integer_type, text_type};
        const StaticResultContext *roots[2];
        for (size_t ordinal = 0; ordinal < 2; ordinal++) {
            size_t index = reverse ? 1 - ordinal : ordinal;
            StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = values[index]};
            const StaticTerm *owner;
            const StaticTermEnvironment *environment;
            assert(static_term_nominal(&terms, caller_module, &input, 1, (SourceSpan){0}, &owner));
            assert(static_term_environment(&terms, caller_module, owner, &environment));
            roots[index] = publication_context(environment);
        }
        assert(static_closure_resolve(&closure) && closure.closed);
        assert(roots[0]->registry_use->candidate != roots[1]->registry_use->candidate);
        assert(field_type(roots[0], "input") == integer_type && field_type(roots[1], "input") == text_type);
        size_t own_bodies = 0;
        for (const StaticClosureEntry *entry = closure.first; entry; entry = entry->next) {
            if (entry->representative == entry && entry->scope->environment->definition == caller_module) { own_bodies++; }
        }
        assert(own_bodies == 2);
        destroy_scheduler();
    }
}

/** A ready named export supplies its Type; the static producer has no runtime body entry. */
static void it_requires_exported_types_without_reinterpreting_static_producers_as_instances(void) {
    const StaticResultContext *root = setup_scheduler("@!\nvalue\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n//integer\n");
    assert(static_closure_resolve(&closure) && closure.closed);
    assert(field_type(root, "value") == integer_type && publications.count > 0);
    for (const StaticClosureEntry *entry = closure.first; entry; entry = entry->next) {
        assert(entry->scope->environment->definition != target_module);
    }
    destroy_scheduler();
}

/** Changing-key recursion must fail before another repeated ordinary body expands. */
static void it_rejects_changing_key_creation_through_ordinary_call_equations(void) {
    (void)setup_scheduler("@<\ninput\n.slot\n@!\nanchor\n//integer\ninput\n anchor\ntext\n//text\nchild&\n/test/fixture/v1.0.0/src/main\nchild& !\n < text\n", "@!\n");
    assert(!static_closure_resolve(&closure) && !closure.closed);
    assert(guards.status == STATIC_GUARD_CONFLICT && guards.rejected_origin == caller_module);
    assert(closure.status == STATIC_CLOSURE_CONFLICT && diagnostic_valid(&closure.error));
    destroy_scheduler();
}

/** Fail each allocation in a full constant-inference closure, including namespace
 * proof publication, body scheduling, normalized keys and solver scratch. */
static void it_unwinds_every_whole_worklist_resolution_allocation_refusal(void) {
    const char *callers[] = {
        "@!\nreceived\n /test/fixture/v1.0.0/src/other/value\n",
        "@!\nknown\n//integer\nworker\n/test/fixture/v1.0.0/src/other\nworker !\n < known\n > fresh\n",
        "@<\ninput\n.slot\n@!\nanchor\n//integer\ninput\n anchor\nself&\n/test/fixture/v1.0.0/src/main\n &\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/other\n .out\n"};
    const char *targets[] = {
        "@=\nvalue\n//integer\n |7\n@!\n",
        "@<\ninput\n.slot\n@>\noutput\n.slot\n@!\noutput\n input\n",
        "@!\n",
        "@...\n.out\n//integer\n"};
    for (size_t phase = 0; phase < sizeof(callers) / sizeof(*callers); phase++) {
        size_t total = 0;
        for (size_t cut = 0; cut <= total; cut++) {
            (void)setup_scheduler(callers[phase], targets[phase]);
            size_t before = allocation_count;
            if (cut) { failure_at = before + cut; }
            bool success = static_closure_resolve(&closure);
            if (!cut) { assert(success && closure.closed); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && !closure.closed && closure.status == STATIC_CLOSURE_OUT_OF_MEMORY && diagnostic_valid(&closure.error)); }
            failure_at = 0;
            destroy_scheduler();
        }
        printf("whole-worklist phase %zu allocation refusal positions: %zu\n", phase, total);
    }
}

/** A later phase refusal or dependency failure must invalidate a previous
 * aggregate certificate without changing the refused output report. Collection
 * is a preparation phase; only a new complete resolution can certify it again. */
static void it_invalidates_previous_closure_on_phase_refusal_or_collection(void) {
    for (size_t phase = 0; phase < 3; phase++) {
        (void)setup_scheduler("@!\nanchor&\n//integer\n", "@!\n");
        assert(static_closure_resolve(&closure) && closure.closed);
        StaticTerm copied = *integer_type;
        if (phase == 0) {
            assert(!static_closure_step(&closure, NULL));
            assert(closure.status == STATIC_CLOSURE_INVALID);
        } else if (phase == 1) {
            assert(!static_constraint_equate(&solver, &copied, integer_type, (SourceSpan){0}));
            StaticClosureStep report = {.types_closed = true};
            assert(!static_closure_step(&closure, &report));
            assert(!report.changed && report.types_closed && closure.status == STATIC_CLOSURE_INVALID);
        } else {
            bool changed = true;
            assert(static_closure_collect(&closure, &changed) && !changed && !closure.closed);
            assert(static_closure_resolve(&closure) && closure.closed);
        }
        if (phase < 2) { assert(!closure.closed && diagnostic_valid(&closure.error)); }
        destroy_scheduler();
    }
}

/** Whole-worklist inference is not allowed to certify ordinary candidate checks. */
static void run(void) {
    it_closes_generic_calls_and_requires_their_actual_definitions();
    it_infers_callee_output_types_from_required_body_equations();
    it_closes_direct_and_saved_constant_receivers_without_value_execution();
    it_rejects_bad_required_definitions_behind_empty_references();
    it_rejects_unanchored_required_types_instead_of_guessing();
    it_closes_same_key_reference_cycles_without_repeated_body_expansion();
    it_preserves_independent_root_acceptance_in_both_discovery_orders();
    it_requires_exported_types_without_reinterpreting_static_producers_as_instances();
    it_rejects_changing_key_creation_through_ordinary_call_equations();
    it_unwinds_every_whole_worklist_resolution_allocation_refusal();
    it_invalidates_previous_closure_on_phase_refusal_or_collection();
}
/** Counted teardown runs for successful and refused fixed points alike. */
int main(void) { run(); return 0; }
