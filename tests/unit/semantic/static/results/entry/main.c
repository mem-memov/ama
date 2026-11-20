/** @file
 * @brief Verify actual producer entry, delayed repeated keys and every partial-entry allocation refusal.
 */
#include "../producers/common.c"

/** Attach one explicitly demanded root; no module instance or output is evaluated. */
static const StaticResultContext *entry_root(const StaticTermEnvironment *environment) {
    const StaticCreationPath *path;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, environment, &path));
    assert(static_result_context(&results, path, &context));
    return context;
}

/** Find a source statement only in its actual lexical module, independent of
 * any matching expression in the caller or of definition discovery order.
 */
static const AstTypeExpression *entry_statement(const StaticModule *module) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->statement->kind == AST_STATIC_PROJECTION &&
            site->statement->as.static_projection == site->expression) { return site->expression; }
    }
    assert(false); return NULL;
}

/** A nonrepeated symbolic producer may expose a guarded scope for equations;
 * actual independent evidence later closes its key without replacing the use.
 */
static void it_retains_one_guarded_producer_scope_until_all_inputs_close(void) {
    setup_producer_requests(request_caller, request_target);
    const StaticResultContext *root = entry_root(caller_environment);
    StaticResultUse *use;
    assert(static_result_use(&results, root, result_projection(0), NULL, &use));
    StaticApplicationOutcome outcome;
    assert(static_result_apply(&results, use, &outcome) && outcome == STATIC_APPLY_PENDING);
    assert(use->producer && use->producer->registry_use->candidate == NULL);
    assert(use->prepared == NULL && use->application == NULL && use->producer_request && use->requests);
    assert(use->source->value == NULL && use->producer->creation->owner->value == NULL);
    assert(guards.path_count == 2 && results.context_count == 2 && result_registry.candidate_count == 1);
    const StaticResultContext *original = use->producer;
    for (size_t index = 0; index < use->producer_request->input_count; index++) {
        assert(static_constraint_equate(&solver, use->producer_request->inputs[index].value, integer_type,
            use->source->expression->span));
    }
    assert(static_result_apply(&results, use, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    assert(use->producer == original && original->registry_use->candidate);
    assert(!original->registry_use->candidate->local_complete &&
        original->registry_use->candidate->state != STATIC_CANDIDATE_CHECKED);
    size_t before = allocation_count;
    assert(static_result_apply(&results, use, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    assert(allocation_count == before && guards.path_count == 2 && result_registry.candidate_count == 2);
    destroy_results();
}

/** Actual forwarding equations are fixture input IR, not an output evaluator. */
static void equate_forwarded_input(StaticResultUse *use) {
    assert(static_result_prepare(&results, use));
    const BindingRequest *request = use->requests->first;
    assert(request && request->direction == BINDING_STATIC_INPUT);
    const StaticResultSource *input;
    assert(static_result_source(&results, use->source->environment, request->syntax.static_binding->input, &input));
    assert(input->head && input->value);
    assert(static_constraint_equate(&solver, input->value, input->head, input->expression->span));
}

/** Unknown A->B->A comparison stops before creating the repeated scope; after
 * real equations close both keys the exact-key graph may close without guessing.
 */
static void it_suspends_unknown_repeated_producers_before_scope_creation_then_closes_equal_keys(void) {
    setup_producer_requests("@...\n.out\n.type\n/test/fixture/v1.0.0/src/other\n .type < .type\n .out > .other out\n.anchor\n//integer\n",
        "@...\n.out\n.type\n/test/fixture/v1.0.0/src/main\n .type < .type\n .out > .main out\n");
    const StaticResultContext *root = entry_root(caller_environment);
    StaticResultUse *first, *back;
    assert(static_result_use(&results, root, entry_statement(caller_module), NULL, &first));
    equate_forwarded_input(first);
    StaticApplicationOutcome outcome;
    assert(static_result_apply(&results, first, &outcome) && outcome == STATIC_APPLY_PENDING && first->producer);
    assert(static_result_use(&results, first->producer, entry_statement(target_module), NULL, &back));
    equate_forwarded_input(back);
    size_t environments = terms.environment_count;
    assert(static_result_apply(&results, back, &outcome) && outcome == STATIC_APPLY_PENDING);
    assert(back->producer == NULL && terms.environment_count == environments && guards.path_count == 2);
    assert(result_registry.candidate_count == 0);
    const StaticTerm *type;
    assert(static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &type));
    assert(static_constraint_equate(&solver, type, integer_type, type->site));
    assert(static_result_apply(&results, back, &outcome) && outcome == STATIC_APPLY_RESOLVED && back->producer);
    StaticRegistryKeyState keys;
    assert(static_registry_refresh_keys(&result_registry, &keys) && keys == STATIC_REGISTRY_KEY_CLOSED);
    assert(back->producer->creation->vertex == root->creation->vertex);
    assert(back->producer->registry_use->candidate == root->registry_use->candidate);
    assert(guards.path_count == 3 && guards.vertex_count == 2 && result_registry.candidate_count == 2);
    destroy_results();
}

/** Distinct concrete keys of the same origin are refused before the repeated
 * environment is interned, even though the immediate parent has another origin.
 */
static void it_refuses_a_different_repeated_key_before_publishing_a_scope(void) {
    setup_producer_requests("@...\n.out\n.type\n/test/fixture/v1.0.0/src/other\n .type < .type\n .out > .other out\n.anchor\n//integer\n",
        "@...\n.out\n.type\n/test/fixture/v1.0.0/src/main\n .type < //text\n .out > .main out\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, caller_module, &input, 1, &environment));
    const StaticResultContext *root = entry_root(environment);
    StaticResultUse *first, *back;
    assert(static_result_use(&results, root, entry_statement(caller_module), NULL, &first));
    equate_forwarded_input(first);
    StaticApplicationOutcome outcome;
    assert(static_result_apply(&results, first, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    assert(static_result_use(&results, first->producer, entry_statement(target_module), NULL, &back));
    equate_forwarded_input(back);
    outcome = STATIC_APPLY_PENDING;
    assert(!static_result_apply(&results, back, &outcome));
    assert(outcome == STATIC_APPLY_PENDING && back->producer == NULL);
    /* Comparing full closed identities may intern normalized key metadata, but
     * must not intern the rejected request's still-scoped variable input map. */
    for (const StaticTermEnvironment *item = terms.first_environment; item; item = item->next) {
        assert(item->definition != caller_module || !item->producer_applied ||
            item->producer_inputs->value != back->producer_request->inputs[0].value);
    }
    assert(results.status == STATIC_RESULT_CONFLICT && guards.status == STATIC_GUARD_CONFLICT);
    assert(guards.path_count == 2 && diagnostic_valid(&results.error));
    destroy_results();
}

/** All partial scope, guard-graph, registry-name and context records are owned
 * after every observed allocation refusal; failed public outcome stays unchanged.
 */
static void it_releases_every_partial_entry_and_registry_allocation_on_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_producer_requests(request_caller, request_target);
        const StaticResultContext *root = entry_root(caller_environment);
        StaticResultUse *use;
        assert(static_result_use(&results, root, result_projection(0), NULL, &use));
        assert(static_result_prepare(&results, use));
        for (size_t index = 0; index < use->producer_request->input_count; index++) {
            assert(static_constraint_equate(&solver, use->producer_request->inputs[index].value, integer_type,
                use->source->expression->span));
        }
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        StaticApplicationOutcome output = STATIC_APPLY_PENDING;
        bool success = static_result_apply(&results, use, &output);
        if (!failure) { assert(success && output == STATIC_APPLY_RESOLVED); total = allocation_count - before; assert(total); }
        else { assert(!success && output == STATIC_APPLY_PENDING && results.status == STATIC_RESULT_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_results();
    }
}

/** Execute actual guarded source preparation and independent-key refinement. */
static void run(void) {
    it_retains_one_guarded_producer_scope_until_all_inputs_close();
    it_suspends_unknown_repeated_producers_before_scope_creation_then_closes_equal_keys();
    it_refuses_a_different_repeated_key_before_publishing_a_scope();
    it_releases_every_partial_entry_and_registry_allocation_on_refusal();
}
/** Return after counted unconditional owner teardown. */
int main(void) { run(); return 0; }
