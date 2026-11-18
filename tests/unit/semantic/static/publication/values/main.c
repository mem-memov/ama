/** @file
 * @brief Verify immutable alias reuse, partial returned families and scoped publication identities.
 */
#include "../common.c"

/** Equal repeated receivers share one immutable lexical alias while producer
 * source occurrences retain separate publication and required-path identities.
 */
static void it_reuses_equal_aliases_without_merging_producer_occurrences(void) {
    setup_publications("@!\nanchor&\n..integer\nfirst&\n.other\n ...slot < ..integer\n ...out > ...alias\nsecond&\n.other\n ...slot < ..integer\n ...out > ...alias\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *first, *second;
    assert(static_publication_resolve(&publications, context, result_field("first"), &first));
    assert(static_publication_resolve(&publications, context, result_field("second"), &second));
    assert(first != second && first->value == second->value && publications.count == 2 && publications.alias_count == 1);
    assert(publications.first_alias->symbol == result_named(caller_module, "...alias"));
    assert(publications.first_alias->value == integer_type && first->first_output->value == integer_type);
    destroy_publications();
}

/** The producer has no residual inputs, but its returned family intentionally
 * retains its own formal and remains usable in another ordinary type expression.
 */
static void it_publishes_partial_returned_families_after_closing_their_producer(void) {
    setup_publications("@!\nanchor&\n..integer\nfactory&\n.other\n ...slot < ..integer\n ...family > ...family alias\nfield&\n...family alias\n ... < ..integer\n", "@<\n...slot\n@>\n...family\n.other\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *factory, *field;
    assert(static_publication_resolve(&publications, context, result_field("factory"), &factory));
    assert(factory->value->as.nominal.residual_count == 0 && factory->first_output->value == target_constructor);
    assert(publications.first_alias->value->as.nominal.residual_count == 1);
    assert(static_publication_resolve(&publications, context, result_field("field"), &field));
    assert(field->value->as.nominal.origin == target_module && field->value->as.nominal.residual_count == 0);
    assert(field->value->as.nominal.first_capture->value == integer_type);
    destroy_publications();
}

/** Same source symbols in different complete lexical owner Types do not capture
 * each other's values or become one publication merely because names match.
 */
static void it_preserves_independent_applied_owner_publications(void) {
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    const StaticTermEnvironment *first_environment = result_applied_environment();
    StaticTermCapture target_capture = {.formal = application_formal(target_module, 0), .value = integer_type};
    const StaticTerm *target, *owner;
    assert(static_term_nominal(&terms, target_module, &target_capture, 1, (SourceSpan){0}, &target));
    StaticTermCapture captures[] = {{.formal = application_formal(caller_module, 0), .value = target},
        {.formal = application_formal(caller_module, 1), .value = target_constructor}};
    assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &owner));
    const StaticTermEnvironment *second_environment;
    assert(static_term_environment(&terms, caller_module, owner, &second_environment));
    const StaticResultContext *first_context = publication_context(first_environment);
    const StaticResultContext *second_context = publication_context(second_environment);
    const StaticSymbol *symbol = result_named(caller_module, "...first");
    const StaticResultDefinition *definition;
    assert(static_result_definition(&results, first_environment, symbol, &definition));
    const StaticPublication *first, *second;
    assert(static_publication_resolve(&publications, first_context, definition->source->expression, &first));
    assert(static_publication_resolve(&publications, second_context, definition->source->expression, &second));
    assert(first != second && first->value == integer_type && second->value == target && publications.count == 2);
    destroy_publications();
}

/** Every static result is canonicalized even when all output actions are skips;
 * no alias descriptors or runtime instances are manufactured for discarded values.
 */
static void it_publishes_every_declared_output_even_when_all_receivers_are_skips(void) {
    setup_publications("@!\nanchor&\n..integer\nfield&\n.other\n ... >\n ... >\n", "@>\n...first\n..integer\n...second\n..integer\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_field("field"), &publication));
    assert(publication->output_count == 2 && publications.alias_count == 0);
    assert(publication->first_output->symbol == result_named(target_module, "...first"));
    assert(publication->first_output->next->symbol == result_named(target_module, "...second"));
    assert(publication->first_output->value == integer_type && publication->first_output->next->value == integer_type);
    destroy_publications();
}

/** A separate unresolved root does not block this application's independently
 * complete producer. Whole-project admission retains that other root obligation.
 */
static void it_closes_selected_producers_without_hiding_or_requiring_unrelated_roots(void) {
    setup_publications("@!\nanchor&\n..integer\nopen&\n.other\nfield&\n.other\n ...slot < ..integer\n ...out > ...alias\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate(&results, context, result_field("open"), &outcome));
    StaticResultUse *open;
    assert(static_result_use(&results, context, result_field("open"), NULL, &open) && !open->closed);
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_field("field"), &publication));
    assert(publication->first_output->value == integer_type && !open->closed);
    StaticConstraintOutcome all;
    assert(static_constraint_solve(&solver, &all) && all == STATIC_CONSTRAINT_PENDING);
    destroy_publications();
}

/** A repeated received alias must agree exactly; failure cannot mutate a prior
 * immutable descriptor or substitute the second producer's selected Type.
 */
static void it_rejects_conflicting_existing_alias_values_without_rebinding(void) {
    setup_publications("@!\nanchor&\n..integer\nfirst&\n.other\n ...slot < ..integer\n ...out > ...alias\nsecond&\n.other\n ...slot < .main\n ...out > ...alias\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *first, *second = NULL;
    assert(static_publication_resolve(&publications, context, result_field("first"), &first));
    assert(!static_publication_resolve(&publications, context, result_field("second"), &second));
    assert(second == NULL && publications.status == STATIC_PUBLICATION_CONFLICT && publications.count == 1);
    assert(first->first_output->value == integer_type && publications.first_alias->value == integer_type);
    destroy_publications();
}

/** Omitted inputs of two written output phases are distinct scoped holes even
 * though they refer to the same original formal in the same caller environment.
 */
static void it_keeps_omitted_inputs_independent_between_explicit_output_phases(void) {
    setup_publications("@>\n...integer expected\n..integer\n...module expected\n.main\n@!\nanchor&\n..integer\nfirst&\n.other\n ...out > ...integer expected\nsecond&\n.other\n ...out > ...module expected\n", result_target_source);
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *first, *second;
    assert(static_publication_resolve(&publications, context, result_field("first"), &first));
    assert(static_publication_resolve(&publications, context, result_field("second"), &second));
    assert(first->value != second->value && first->value->as.nominal.origin == target_module);
    assert(second->value->as.nominal.origin == target_module);
    assert(first->value->as.nominal.first_capture->value == integer_type);
    assert(second->value->as.nominal.first_capture->value == caller_constructor);
    assert(first->use->application->first->generated && second->use->application->first->generated);
    assert(first->use->application->first->value != second->use->application->first->value);
    destroy_publications();
}

/** A constant alias value cannot hide the original producer's unused unknown
 * input when the name is later reused as an existing receiver.
 */
static void it_requires_original_receiver_alias_producers_to_close_before_reuse(void) {
    for (size_t complete = 0; complete < 2; complete++) {
        setup_publications(complete ?
            "@!\nanchor&\n..integer\nfirst&\n.other\n ...slot < ..integer\n ...out > ...alias\nsecond&\n.other\n ...slot < ..integer\n ...out > ...alias\n" :
            "@!\nanchor&\n..integer\nfirst&\n.other\n ...out > ...alias\nsecond&\n.other\n ...slot < ..integer\n ...out > ...alias\n",
            "@<\n...slot\n@>\n...out\n..integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticPublication *publication = NULL;
        bool success = static_publication_resolve(&publications, context, result_field("second"), &publication);
        assert(success == (complete != 0));
        if (success) { assert(publication->first_output->value == integer_type && publications.alias_count == 1); }
        else { assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED && publications.alias_count == 0); }
        destroy_publications();
    }
}

/** A cached intermediate alias retains its RHS proof. Reaching a constant value
 * through that cache cannot conceal an earlier incomplete receiver producer.
 */
static void it_follows_cached_receiver_rhs_dependencies_transitively(void) {
    for (size_t complete = 0; complete < 2; complete++) {
        setup_publications(complete ?
            "@!\nanchor&\n..integer\nfirst&\n.other\n ...slot < ..integer\n ...out > ...a\nsecond&\n.other\n ...slot < ..integer\n ...out > ...a\n ...second > ...b\nthird&\n.other\n ...slot < ..integer\n ...out > ...b\n" :
            "@!\nanchor&\n..integer\nfirst&\n.other\n ...out > ...a\nsecond&\n.other\n ...slot < ..integer\n ...out > ...a\n ...second > ...b\nthird&\n.other\n ...slot < ..integer\n ...out > ...b\n",
            "@<\n...slot\n@>\n...out\n..integer\n...second\n..integer\n@!\n");
        const StaticResultContext *context = publication_context(caller_environment);
        const StaticPublication *publication = NULL;
        bool success = static_publication_resolve(&publications, context, result_field("third"), &publication);
        assert(success == (complete != 0));
        bool cached = false;
        const StaticSymbol *symbol = result_named(caller_module, "...b");
        for (const StaticResultCache *entry = results.first_cache; entry; entry = entry->next) {
            if (entry->symbol == symbol) { assert(entry->rhs != NULL); cached = true; }
        }
        assert(cached);
        if (!complete) { assert(publication == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED); }
        destroy_publications();
    }
}

/** Independent result receivers can form a compatible equality cycle. Visiting
 * actual RHS proofs once accepts it without adding false definition edges.
 */
static void it_accepts_anchored_receiver_equality_cycles_without_recursive_closure(void) {
    setup_publications("@>\n...first\n.other\n ...out > ...second\n...second\n.other\n ...out > ...first\n@!\nanchor&\n..integer\n",
        "@>\n...out\n.other\n@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticResultDefinition *definition;
    assert(static_result_definition(&results, caller_environment, result_named(caller_module, "...first"), &definition));
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, definition->source->expression, &publication));
    assert(publication->value == target_constructor && publication->first_output->value == target_constructor);
    assert(guards.repeated_definition_ancestor == NULL && publications.count == 1);
    destroy_publications();
}

/** Execute immutable identities, closed family returns and complete skip phases. */
static void run(void) {
    it_reuses_equal_aliases_without_merging_producer_occurrences();
    it_publishes_partial_returned_families_after_closing_their_producer();
    it_preserves_independent_applied_owner_publications();
    it_publishes_every_declared_output_even_when_all_receivers_are_skips();
    it_closes_selected_producers_without_hiding_or_requiring_unrelated_roots();
    it_rejects_conflicting_existing_alias_values_without_rebinding();
    it_keeps_omitted_inputs_independent_between_explicit_output_phases();
    it_requires_original_receiver_alias_producers_to_close_before_reuse();
    it_follows_cached_receiver_rhs_dependencies_transitively();
    it_accepts_anchored_receiver_equality_cycles_without_recursive_closure();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
