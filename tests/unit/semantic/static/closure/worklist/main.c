/** @file
 * @brief Verify finite required-context scheduling metadata without candidate admission or runtime merging.
 */
#include "../../connections/common.c"
#include "semantic/static/closure/plan.h"

/** The queue dies before every borrowed context and original source. */
static StaticClosurePlan closure;

/** Initialize the worklist without allocating or selecting any hidden root. */
static void setup_worklist(void) {
    setup_connections("@<\nvalue\n.slot\nsecond value\n.second slot\n@!\nanchor&\n//integer\n", "@!\n");
    size_t before = allocation_count;
    static_closure_init(&closure, &connections, sources.allocator);
    assert(allocation_count == before && closure.status == STATIC_CLOSURE_OK);
}

/** Counted child teardown detects leaked partial entries or scope variables. */
static void destroy_worklist(void) {
    static_closure_destroy(&closure);
    assert(!closure.connections && !closure.first && closure.count == 0);
    destroy_connections();
}

/** Explicit fixture roots supply nominal metadata rather than a default instance. */
static const StaticResultContext *root_with(const StaticTerm *slot) {
    StaticTermCapture inputs[] = {
        {.formal = application_formal(caller_module, 0), .value = slot},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, inputs, 2, (SourceSpan){0}, &owner));
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    return publication_context(environment);
}

/** Equal closed keys share a body representative while retaining both incoming paths. */
static void it_retains_actual_paths_and_shares_only_exact_candidate_identity(void) {
    setup_worklist();
    const StaticResultContext *root = root_with(integer_type);
    const StaticCreationPath *required;
    StaticConstraintOutcome known;
    SourceSpan site = caller_module->first_statement->syntax->span;
    assert(static_guard_require_type(&guards, root->creation, root->creation->owner->value, site, &required, &known));
    const StaticResultContext *other;
    assert(static_result_context(&results, required, &other));
    bool changed;
    assert(static_closure_collect(&closure, &changed) && changed && closure.count == 2);
    assert(closure.first->context != closure.last->context);
    assert(closure.first->representative == closure.first && closure.last->representative == closure.first);
    assert(closure.first->context->registry_use->candidate == closure.last->context->registry_use->candidate);
    assert(result_registry.dependency_count == 1 && guards.edge_count == 0);
    assert(!root->registry_use->candidate->local_started && !root->registry_use->candidate->local_complete);
    assert(static_closure_collect(&closure, &changed) && !changed);
    destroy_worklist();
}

/** Unknown complete keys remain distinct until actual independent equations
 * prove equality, even for the exact same source template and interface. */
static void it_merges_pending_work_only_after_independent_keys_close(void) {
    setup_worklist();
    const StaticTerm *left = application_symbol(".slot");
    const StaticTerm *right = application_symbol(".second slot");
    const AstNode *declaration = caller_module->first_statement->syntax;
    const StaticResultContext *first = root_with(left), *second = root_with(right);
    assert(first != second);
    bool changed;
    assert(static_closure_collect(&closure, &changed) && closure.count == 2);
    assert(closure.first->representative == closure.first && closure.last->representative == closure.last);
    assert(!first->registry_use->candidate && !second->registry_use->candidate);
    assert(static_constraint_equate(&solver, left, integer_type, declaration->span));
    assert(static_constraint_equate(&solver, right, integer_type, declaration->span));
    assert(static_closure_collect(&closure, &changed) && changed);
    assert(closure.last->representative == closure.first);
    assert(first->registry_use->candidate == second->registry_use->candidate);
    assert(static_closure_collect(&closure, &changed) && !changed);
    destroy_worklist();
}

/** Static producer contexts stay in graph publication and create no runtime scope. */
static void it_leaves_static_producer_contexts_out_of_ordinary_body_work(void) {
    setup_connections("@!\nanchor&\n//integer\n", "@...\n.out\n//integer\n");
    static_closure_init(&closure, &connections, sources.allocator);
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, target_module, NULL, 0, &environment));
    (void)publication_context(environment);
    bool changed;
    assert(static_closure_collect(&closure, &changed) && !changed);
    assert(closure.count == 0 && connections.context_count == 0 && connections.scope_count == 0);
    destroy_worklist();
}

/** Missing progress receivers and count overflow preserve caller outputs. */
static void it_refuses_invalid_receivers_and_checked_queue_count_overflow(void) {
    for (size_t kind = 0; kind < 2; kind++) {
        setup_worklist();
        (void)root_with(integer_type);
        bool changed = false;
        if (kind) { closure.count = SIZE_MAX; }
        assert(!static_closure_collect(&closure, kind ? &changed : NULL));
        assert(!changed && closure.first == NULL);
        assert(closure.status == (kind ? STATIC_CLOSURE_OUT_OF_MEMORY : STATIC_CLOSURE_INVALID));
        destroy_worklist();
    }
}

/** Every normalization, scope handoff and queue allocation can refuse without
 * leaking borrowed/partial owners or publishing progress after failure. */
static void it_unwinds_every_required_context_collection_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_worklist();
        (void)root_with(integer_type);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        bool changed = false;
        bool success = static_closure_collect(&closure, &changed);
        if (!cut) { assert(success && changed); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && !changed && closure.status == STATIC_CLOSURE_OUT_OF_MEMORY && diagnostic_valid(&closure.error)); }
        failure_at = 0;
        destroy_worklist();
    }
}

/** Explicitly dispatch identity, phase separation and failure-boundary assertions. */
static void run(void) {
    it_retains_actual_paths_and_shares_only_exact_candidate_identity();
    it_merges_pending_work_only_after_independent_keys_close();
    it_leaves_static_producer_contexts_out_of_ordinary_body_work();
    it_refuses_invalid_receivers_and_checked_queue_count_overflow();
    it_unwinds_every_required_context_collection_allocation_refusal();
}
/** Complete counted teardown before reporting leaf success. */
int main(void) { run(); return 0; }
