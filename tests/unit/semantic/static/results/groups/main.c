/** @file
 * @brief Verify same-key creation groups, anchored inference and unselected-error checks through actual graph passes.
 */
#include "../common.c"

/** Initialize only current source graph fixtures and their explicit actual root. */
static const StaticResultContext *setup_group(const char *caller, const char *target) {
    setup_guards(caller, target);
    initialize_results();
    const StaticCreationPath *path;
    const StaticResultContext *context;
    StaticResultGraph *graph;
    assert(static_creation_root(&guards, caller_environment, &path));
    assert(static_result_context(&results, path, &context));
    assert(static_result_require_graph(&results, context, &graph));
    return context;
}

/** Exact finite evidence counts, not allocation activity or an iteration budget. */
typedef struct { size_t equations, metadata, requirements, contexts, graphs, caches, complete; } GroupEvidence;

/** Snapshot actual inference facts and newly checked graph obligations. */
static GroupEvidence evidence(void) {
    GroupEvidence value = {.equations = solver.equation_count, .metadata = solver.metadata_count,
        .requirements = solver.requirement_count, .contexts = results.context_count,
        .graphs = results.graph_count, .caches = results.cache_count};
    for (const StaticResultGraph *graph = results.first_graph; graph; graph = graph->next) {
        value.complete += graph->local_complete;
    }
    return value;
}

/** Compare named evidence components, never struct padding or allocation IDs. */
static bool same_evidence(GroupEvidence a, GroupEvidence b) {
    return a.equations == b.equations && a.metadata == b.metadata && a.requirements == b.requirements &&
        a.contexts == b.contexts && a.graphs == b.graphs && a.caches == b.caches && a.complete == b.complete;
}

/** Drain new required witnesses in one pass; each value evaluation itself uses
 * the explicit guarded stack. No source-language depth budget is imposed.
 */
static bool pass_group(bool *complete) {
    bool all = true;
    for (StaticResultGraph *graph = results.first_graph; graph; graph = graph->next) {
        StaticResultOutcome outcome;
        if (!static_result_graph_pass(&results, graph, &outcome)) { return false; }
        all &= outcome == STATIC_RESULT_READY;
    }
    *complete = all;
    return true;
}

/** Positive fixtures must close or add genuine finite evidence before retrying. */
static void settle_group(void) {
    for (;;) {
        GroupEvidence before = evidence();
        bool complete;
        assert(pass_group(&complete));
        if (complete) { return; }
        assert(!same_evidence(before, evidence()));
    }
}

/** Read actual solved metadata independently of proof-cache/context identity. */
static const StaticTerm *group_value(const StaticTermEnvironment *environment, const char *name) {
    const StaticTerm *term, *value;
    StaticConstraintOutcome outcome;
    assert(static_term_symbol(&terms, environment, result_named(environment->definition, name), &term));
    assert(static_constraint_value(&solver, term, &value, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    return value;
}

/** A.out -> B.selected and B.unselected -> A.out is an acyclic value graph,
 * although whole-producer validation has a permitted A -> B -> A cycle.
 */
static void it_closes_same_key_validation_cycles_without_inventing_value_definition_cycles(void) {
    const StaticResultContext *root = setup_group("@...\n/test/fixture/v1.0.0/src/other\n .selected > .out\n",
        "@...\n.selected\n//integer\n/test/fixture/v1.0.0/src/main\n .out > .unselected\n");
    settle_group();
    assert(results.graph_count == 2 && group_value(caller_environment, ".out") == integer_type);
    const StaticResultGraph *other = results.first_graph->next;
    assert(other && other->context->creation->owner->environment->definition == target_module);
    assert(group_value(other->context->creation->owner->environment, ".unselected") == integer_type);
    assert(root->registry_use->candidate->local_complete && other->context->registry_use->candidate->local_complete);
    assert(guards.status == STATIC_GUARD_OK && guards.repeated_definition_ancestor == NULL);
    StaticRegistryCheckState checks;
    assert(static_registry_refresh_checks(&result_registry, &checks) && checks == STATIC_REGISTRY_CHECKS_PENDING);
    /* Ordinary integer Type checking is a retained later admission obligation. */
    destroy_results();
}

/** Pair-declared receiver evidence determines omitted producer inputs through
 * ordinary structural equations without an artificial value-definition cycle.
 */
static void it_solves_anchored_receiver_equations_across_all_required_graphs(void) {
    (void)setup_group("@...\n.known\n//integer\n/test/fixture/v1.0.0/src/other\n .out > .known\n",
        "@...\n.out\n.type\n");
    settle_group();
    assert(results.graph_count == 2 && guards.definition_edge_count == 0);
    const StaticResultGraph *other = results.first_graph->next;
    assert(other->context->registry_use->candidate && other->local_complete);
    assert(group_value(caller_environment, ".known") == integer_type);
    assert(group_value(other->context->creation->owner->environment, ".out") == integer_type);
    assert(caller_module->parameter_count == 0);
    destroy_results();
}

/** Available integer metadata and a fresh alias supply no input anchor. Stable
 * Pending is retained without picking a type or manufacturing another scope.
 */
static void it_leaves_unanchored_inputs_pending_without_fabricated_defaults_or_progress(void) {
    (void)setup_group("@...\n/test/fixture/v1.0.0/src/other\n .out > .fresh\n.anchor\n//integer\n",
        "@...\n.out\n.type\n");
    for (;;) {
        GroupEvidence before = evidence();
        bool complete;
        assert(pass_group(&complete) && !complete);
        if (same_evidence(before, evidence())) { break; }
    }
    GroupEvidence stable = evidence();
    bool complete;
    assert(pass_group(&complete) && !complete && same_evidence(stable, evidence()));
    assert(results.status == STATIC_RESULT_OK && caller_module->parameter_count == 0);
    const StaticResultGraph *other = results.first_graph->next;
    assert(other && other->context->registry_use->candidate == NULL && !other->local_complete);
    destroy_results();
}

/** Checking a known selected output never excuses an illegal unselected public
 * definition. Its real local cycle is discovered when that graph is drained.
 */
static void it_rejects_an_unselected_definition_cycle_even_after_a_selected_value_is_known(void) {
    const StaticResultContext *root = setup_group("@...\n/test/fixture/v1.0.0/src/other\n .good > .received\n",
        "@...\n.good\n//integer\n.bad a\n.bad b\n.bad b\n.bad a\n");
    StaticResultOutcome outcome;
    assert(static_result_graph_pass(&results, results.first_graph, &outcome));
    assert(group_value(root->creation->owner->environment, ".received") == integer_type);
    StaticResultGraph *other = results.first_graph->next;
    assert(other && !static_result_graph_pass(&results, other, &outcome));
    assert(results.status == STATIC_RESULT_CONFLICT && guards.repeated_definition_ancestor);
    assert(diagnostic_valid(&results.error));
    destroy_results();
}

/** Run finite group, equality inference, underdetermination and complete-output checks. */
static void run(void) {
    it_closes_same_key_validation_cycles_without_inventing_value_definition_cycles();
    it_solves_anchored_receiver_equations_across_all_required_graphs();
    it_leaves_unanchored_inputs_pending_without_fabricated_defaults_or_progress();
    it_rejects_an_unselected_definition_cycle_even_after_a_selected_value_is_known();
}
/** Return only after counted source/name/registry/graph teardown. */
int main(void) { run(); return 0; }
