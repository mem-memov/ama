/** @file
 * @brief Verify exact receiver inference, unresolved producer rejection and immutable static results.
 */
#include "../common.c"

/** A receiver's independently declared result Type closes an omitted producer
 * input. Its definition is a root, not a fictitious dependency from the producer.
 */
static void it_closes_omitted_inputs_from_independent_existing_result_receivers(void) {
    setup_publications("@>\n...expected\n..integer\n@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...expected\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication;
    bool success = static_publication_resolve(&publications, context, result_field("field"), &publication);
    if (!success) { fprintf(stderr, "Publication failure: %s\n", publications.error.message); }
    assert(success && publication->value->as.nominal.origin == target_module);
    assert(publication->value->as.nominal.first_capture->value == integer_type);
    assert(publication->output_count == 1 && publication->first_output->value == integer_type);
    assert(publications.alias_count == 0 && guards.repeated_definition_ancestor == NULL);
    const StaticPublication *again;
    assert(static_publication_resolve(&publications, context, result_field("field"), &again));
    assert(again == publication && publications.count == 1);
    destroy_publications();
}

/** Neither a new alias nor an output skip anchors a missing identity input;
 * even a known constant result cannot determine an unused producer parameter.
 */
static void it_rejects_fresh_aliases_skips_and_unused_input_underdetermination(void) {
    const char *callers[] = {
        "@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...alias\n",
        "@!\nanchor&\n..integer\nfield&\n.other\n ... >\n",
        "@>\n...expected\n..integer\n@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...expected\n"
    };
    for (size_t mode = 0; mode < 3; mode++) {
        setup_publications(callers[mode], mode == 2 ? "@<\n...slot\n@>\n...out\n..integer\n@!\n" : result_target_source);
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticPublication *publication = NULL;
        assert(!static_publication_resolve(&publications, context, result_field("field"), &publication));
        assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
        assert(publications.count == 0 && publications.alias_count == 0 && diagnostic_valid(&publications.error));
        destroy_publications();
    }
}

/** Publication retains returned constructor origin and local residuals, while
 * requiring all producer captures, including otherwise unused family arguments.
 */
static void it_publishes_closed_producers_returning_partial_constructors_in_applied_scopes(void) {
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    const StaticTermEnvironment *applied = result_applied_environment();
    const StaticResultContext *context = publication_context(applied);
    const StaticResultDefinition *definition;
    assert(static_result_definition(&results, applied, result_named(caller_module, "...partial"), &definition));
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, definition->source->expression, &publication));
    assert(publication->value == target_constructor && publication->value->as.nominal.residual_count == 1);
    assert(publication->output_count == 0 && publications.alias_count == 0);
    destroy_publications();
}

/** An output that preserves only one of two inputs cannot determine the other.
 * Supplying the lost input explicitly leaves one uniquely recoverable value.
 */
static void it_refuses_to_invent_inputs_lost_by_a_noninvertible_output(void) {
    for (size_t supplied = 0; supplied < 2; supplied++) {
        setup_publications(supplied ?
            "@>\n...expected\n..integer\n@!\nanchor&\n..integer\nfield&\n.other\n ...key < ..integer\n ...out > ...expected\n" :
            "@>\n...expected\n..integer\n@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...expected\n",
            "@<\n...key\n...value\n@>\n...out\n...value\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticPublication *publication = NULL;
        bool success = static_publication_resolve(&publications, context, result_field("field"), &publication);
        assert(success == (supplied != 0));
        if (success) {
            assert(publication->value->as.nominal.capture_count == 2);
            assert(publication->value->as.nominal.first_capture->value == integer_type);
            assert(publication->value->as.nominal.last_capture->value == integer_type);
        } else { assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED); }
        destroy_publications();
    }
}

/** A known selected receiver does not identify the unknown producer origin.
 * Formal names/signature alone do not justify choosing a filesystem module.
 */
static void it_rejects_unknown_producer_origins_even_with_known_selected_receivers(void) {
    setup_publications("@<\n...factory\n ...slot\n@>\n...expected\n..integer\n@!\nanchor&\n..integer\nfield&\n...factory\n ...slot < ..integer\n ...out > ...expected\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication = NULL;
    assert(!static_publication_resolve(&publications, context, result_field("field"), &publication));
    assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
    assert(publications.count == 0 && publications.alias_count == 0);
    destroy_publications();
}

/** Known value equations do not manufacture a required source context. A first
 * attempt can register that actual context without adding any new value facts;
 * it must then recheck the previously missing foreign definition provenance.
 */
static void it_treats_new_checked_source_contexts_as_real_provenance_evidence(void) {
    setup_publications("@<\n...type\n@!\nanchor&\n..integer\nfield&\n...type\n ...out > ...alias\n",
        "@>\n...out\n.other\n@!\n");
    const StaticTermEnvironment *foreign_environment;
    assert(static_term_environment(&terms, target_module, target_constructor, &foreign_environment));
    const StaticSymbol *foreign_symbol = result_named(target_module, "...out");
    const StaticTerm *foreign;
    assert(static_term_symbol(&terms, foreign_environment, foreign_symbol, &foreign));
    StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = foreign};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &owner));
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    const StaticResultContext *context = publication_context(environment);
    const StaticResultDefinition *definition;
    const StaticResultSource *field;
    assert(static_result_definition(&results, foreign_environment, foreign_symbol, &definition));
    assert(static_result_source(&results, environment, result_field("field"), &field));
    const StaticTerm *alias;
    assert(static_term_symbol(&terms, environment, result_named(caller_module, "...alias"), &alias));
    assert(static_constraint_equate(&solver, foreign, target_constructor, foreign_symbol->site));
    assert(static_constraint_equate(&solver, definition->source->value, target_constructor, foreign_symbol->site));
    assert(static_constraint_equate(&solver, foreign, definition->source->value, foreign_symbol->site));
    assert(static_constraint_equate(&solver, field->value, target_constructor, foreign_symbol->site));
    assert(static_constraint_equate(&solver, foreign, alias, foreign_symbol->site));
    assert(static_constraint_require(&solver, target_constructor, foreign_symbol->site));
    StaticConstraintOutcome solved;
    assert(static_constraint_solve(&solver, &solved));
    size_t contexts = results.context_count;
    const StaticPublication *publication = NULL;
    bool success = static_publication_resolve(&publications, context, result_field("field"), &publication);
    if (!success) { fprintf(stderr, "Provenance closure failure: %s\n", publications.error.message); }
    assert(success && publication->value == target_constructor && results.context_count > contexts);
    destroy_publications();
}

/** Execute independent anchors, missing producer inputs and returned residuals. */
static void run(void) {
    it_closes_omitted_inputs_from_independent_existing_result_receivers();
    it_rejects_fresh_aliases_skips_and_unused_input_underdetermination();
    it_publishes_closed_producers_returning_partial_constructors_in_applied_scopes();
    it_refuses_to_invent_inputs_lost_by_a_noninvertible_output();
    it_rejects_unknown_producer_origins_even_with_known_selected_receivers();
    it_treats_new_checked_source_contexts_as_real_provenance_evidence();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
