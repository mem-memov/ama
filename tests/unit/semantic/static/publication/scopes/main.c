/** @file
 * @brief Check full named export bundles without producer Types or synthetic calls.
 */
#include "../common.c"

/** A root publishes both pair-defined and forwarded public names in source order. */
static void it_publishes_all_public_names_and_reuses_the_actual_root(void) {
    setup_publications("@...\n...own\n..integer\n.other\n ...out > ...forwarded\n", "@...\n...out\n..integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *first, *again;
    assert(static_publication_scope(&publications, context, &first));
    assert(first->use == NULL && first->value == NULL && first->context == context);
    assert(first->output_count == 2 && first->first_output->value == integer_type);
    assert(first->first_output->symbol == result_named(caller_module, "...own"));
    assert(first->first_output->next->symbol == result_named(caller_module, "...forwarded"));
    assert(first->first_output->next->value == integer_type);
    assert(static_publication_scope(&publications, context, &again) && again == first);
    assert(publications.count == 1);
    destroy_publications();
}

/** The complete input key can be inferred from an existing receiver equation. */
static void it_infers_omitted_inputs_and_publishes_a_valueless_producer_bundle(void) {
    setup_publications("@...\n...known\n..integer\n.other\n ...out > ...known\n", "@...\n...out\n...type\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_projection(0), &publication));
    assert(publication->value == NULL && publication->context != NULL);
    assert(publication->output_count == 1 && publication->first_output->value == integer_type);
    assert(publication->context->registry_use->key_state == STATIC_REGISTRY_KEY_CLOSED);
    destroy_publications();
}

/** Validation cycles do not manufacture value-definition cycles. */
static void it_closes_cyclic_validation_with_acyclic_export_definitions(void) {
    setup_publications("@...\n.other\n ...selected > ...out\n",
        "@...\n...selected\n..integer\n.main\n ...out > ...unselected\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication;
    assert(static_publication_scope(&publications, context, &publication));
    assert(publication->output_count == 1 && publication->first_output->value == integer_type);
    assert(results.graph_count == 2);
    destroy_publications();
}

/** A selected known result cannot hide an unselected recursive value. */
static void it_rejects_an_unselected_value_cycle_without_exposing_partial_outputs(void) {
    setup_publications("@...\n.other\n ...good > ...selected\n",
        "@...\n...good\n..integer\n...bad\n...bad\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication = (const StaticPublication *)&publications;
    assert(!static_publication_resolve(&publications, context, result_projection(0), &publication));
    assert(publication == (const StaticPublication *)&publications && publications.count == 0);
    assert(publications.status == STATIC_PUBLICATION_CONFLICT);
    destroy_publications();
}

/** An underdetermined key stays an error; available concrete types are not defaults. */
static void it_refuses_unanchored_producers_at_a_stable_fixed_point(void) {
    setup_publications("@...\n.other\n ...out > ...unknown\n", "@...\n...out\n...type\n...anchor\n..integer\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication = NULL;
    assert(!static_publication_scope(&publications, context, &publication));
    assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
    assert(caller_module->parameter_count == 0 && publications.count == 0);
    destroy_publications();
}

/** Only actual registered static roots may publish bundles. Public receiver
 * failure and count overflow leave all output pointers unchanged.
 */
static void it_refuses_invalid_root_handles_ordinary_roots_and_count_overflow(void) {
    for (size_t kind = 0; kind < 4; kind++) {
        setup_publications(kind == 2 ? "@!\nanchor&\n..integer\n" :
            "@...\n...out\n..integer\n", "@...\n");
        const StaticResultContext *context = publication_context(caller_environment);
        StaticResultContext copied = *context;
        const StaticPublication *publication = NULL;
        if (kind == 3) { publications.count = SIZE_MAX; }
        assert(!static_publication_scope(&publications, kind == 1 ? &copied : context,
            kind == 0 ? NULL : &publication));
        assert(publication == NULL && publications.status ==
            (kind == 3 ? STATIC_PUBLICATION_OUT_OF_MEMORY : STATIC_PUBLICATION_INVALID));
        destroy_publications();
    }
}

/** Explicit harness keeps each specified scenario visible. */
static void run(void) {
    it_refuses_invalid_root_handles_ordinary_roots_and_count_overflow();
    it_publishes_all_public_names_and_reuses_the_actual_root();
    it_infers_omitted_inputs_and_publishes_a_valueless_producer_bundle();
    it_closes_cyclic_validation_with_acyclic_export_definitions();
    it_rejects_an_unselected_value_cycle_without_exposing_partial_outputs();
    it_refuses_unanchored_producers_at_a_stable_fixed_point();
}

/** Return only after all counted owners have been released. */
int main(void) { run(); return 0; }
