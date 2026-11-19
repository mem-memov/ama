/** @file
 * @brief Verify cache-independent ancestry checks and direct/mutual definition-cycle refusal.
 */
#include "../common.c"

/** Warm foreign outputs independently; a cached value must not hide an unknown
 * incoming ancestor when that producer reaches the original module again.
 */
static void it_preserves_unknown_ancestor_obligations_in_cold_and_warm_orders(void) {
    for (size_t warm = 0; warm < 2; warm++) {
        for (size_t known = 0; known < 2; known++) {
            setup_publications("@...\n...out\n...slot\n.other\n ...out > ...received\n",
                "@...\n...out\n..integer\n.main\n ...slot < ..integer\n ...out > ...back\n");
            if (warm) {
                const StaticTermEnvironment *environment;
                assert(static_term_producer_environment(&terms, target_module, NULL, 0, &environment));
                const StaticResultContext *foreign = publication_context(environment);
                const StaticPublication *publication;
                assert(static_publication_scope(&publications, foreign, &publication));
                assert(publication->first_output->value == integer_type);
            }
            const StaticResultContext *context = publication_context(caller_environment);
            if (known) {
                assert(static_constraint_equate(&solver, application_symbol("...slot"), integer_type,
                    result_named(caller_module, "...slot")->site));
            }
            const StaticPublication *publication = NULL;
            bool success = static_publication_resolve(&publications, context, result_projection(0), &publication);
            assert(success == (known != 0));
            if (success) { assert(publication->first_output->value == integer_type); }
            else { assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED); }
            destroy_publications();
        }
    }
}

/** A first B witness under unknown A cannot suppress B's independent root.
 * The second actual path can derive A(integer), complete B and later help the
 * original path once A's independent input evidence becomes available.
 */
static void it_replays_independent_roots_after_a_first_path_with_unknown_ancestry(void) {
    setup_publications("@...\n...out\n...slot\n.other\n ...out > ...received\n",
        "@...\n...out\n..integer\n.main\n ...slot < ..integer\n ...out > ...back\n");
    const StaticResultContext *context = publication_context(caller_environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate(&results, context, result_projection(0), &outcome));
    for (StaticResultGraph *graph = results.first_graph; graph; graph = graph->next) {
        assert(static_result_graph_pass(&results, graph, &outcome));
    }
    assert(context->registry_use->key_state == STATIC_REGISTRY_KEY_PENDING);
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, target_module, NULL, 0, &environment));
    const StaticResultContext *independent = publication_context(environment);
    const StaticPublication *publication;
    assert(static_publication_scope(&publications, independent, &publication));
    assert(publication->context == independent && publication->first_output->value == integer_type);
    assert(context->registry_use->key_state == STATIC_REGISTRY_KEY_PENDING);
    assert(static_constraint_equate(&solver, application_symbol("...slot"), integer_type,
        result_named(caller_module, "...slot")->site));
    assert(static_publication_scope(&publications, context, &publication));
    assert(publication->first_output->value == integer_type);
    destroy_publications();
}

/** Static input maps are specialization identities even for equal selected values. */
static void it_refuses_changed_keys_on_a_repeated_original_producer(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/other type", "@!\n");
    setup_publications("@...\n...out\n...slot\n.main\n ...slot < .other type\n ...out > ...received\n...anchor\n..integer\n", "@...\n");
    const StaticResultContext *context = publication_context(caller_environment);
    assert(static_constraint_equate(&solver, application_symbol("...slot"), integer_type,
        result_named(caller_module, "...slot")->site));
    const StaticPublication *publication = NULL;
    assert(!static_publication_scope(&publications, context, &publication));
    assert(publication == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
    assert(guards.repeated_ancestor == context->creation && guards.rejected_origin == caller_module);
    assert(guards.earlier_creation_key && guards.rejected_creation_key &&
        guards.earlier_creation_key != guards.rejected_creation_key && publications.count == 0);
    destroy_publications();
}

/** No source-value anchor exists in a direct or mutually recursive selected definition. */
static void it_rejects_direct_and_mutual_selected_definition_cycles(void) {
    for (size_t mutual = 0; mutual < 2; mutual++) {
        setup_publications(mutual ? "@...\n.other\n ...out > ...out\n...anchor\n..integer\n" :
            "@...\n.main\n ...out > ...out\n...anchor\n..integer\n",
            "@...\n.main\n ...out > ...out\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticPublication *publication = NULL;
        assert(!static_publication_scope(&publications, context, &publication));
        assert(publication == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
        assert(guards.repeated_definition_ancestor && guards.rejected_definition);
        assert(guards.creation_witness == NULL && publications.count == 0);
        destroy_publications();
    }
}

/** Execute cache orders, full-key recurrence and independent definition graphs. */
static void run(void) {
    it_replays_independent_roots_after_a_first_path_with_unknown_ancestry();
    it_preserves_unknown_ancestor_obligations_in_cold_and_warm_orders();
    it_refuses_changed_keys_on_a_repeated_original_producer();
    it_rejects_direct_and_mutual_selected_definition_cycles();
}
/** Every scenario tears down all counted owners. */
int main(void) { run(); return 0; }
