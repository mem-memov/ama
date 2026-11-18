/** @file
 * @brief Distinguish exact recursive nominal results from cyclic static computations.
 */
#include "../common.c"

/** Main's one Type slot is explicit; each test supplies independent evidence. */
static const char *self_source = "@<\n...slot\n@>\n...out\n.main\n ...slot < ...slot\n"
    "@!\nanchor&\n..integer\nfield&\n.main\n ...slot < ..integer\n ...out > ...alias\n";

/** Register one applied main root without running or constructing an instance. */
static const StaticResultContext *closed_context(void) {
    StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTerm *value;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &value));
    assert(static_term_environment(&terms, caller_module, value, &environment));
    return publication_context(environment);
}

/** A nominal self Type returns one canonical identity without expanding its
 * static result again. Repeated publication uses its original finite proof.
 */
static void it_publishes_the_producers_own_nominal_type_without_value_recursion(void) {
    setup_publications(self_source, "@!\n");
    const StaticResultContext *context = closed_context();
    const StaticPublication *publication, *again;
    assert(static_publication_resolve(&publications, context, result_field("field"), &publication));
    assert(publication->value == context->creation->vertex->value);
    assert(publication->first_output->value == publication->value);
    size_t own_keys = 0;
    bool self_edge = false;
    for (const StaticCreationVertex *vertex = guards.first_vertex; vertex; vertex = vertex->next) {
        if (vertex->value->as.nominal.origin != caller_module) { continue; }
        own_keys++;
        for (const StaticCreationEdge *edge = vertex->first; edge; edge = edge->next) {
            self_edge |= edge->target == vertex;
        }
    }
    assert(own_keys == 1 && self_edge);
    assert(publication->first_output->symbol == result_named(caller_module, "...out"));
    assert(guards.creation_witness == NULL && guards.rejected_definition == NULL);
    size_t paths = guards.path_count, contexts = results.context_count, definitions = results.definition_use_count;
    assert(static_publication_resolve(&publications, context, result_field("field"), &again));
    assert(again == publication && guards.path_count == paths && results.context_count == contexts);
    assert(results.definition_use_count == definitions);
    destroy_publications();
}

/** Captured Template arguments, even unused ones, remain part of the same-key
 * nominal identity. Forwarding one does not expand its definition or residuals.
 */
static void it_preserves_closed_recursive_identity_with_constructor_captures(void) {
    setup_publications("@<\n...type\n...family\n ...slot\n@>\n...out\n.main\n"
        " ...type < ...type\n ...family < ...family\n@!\nanchor&\n..integer\nfield&\n.main\n"
        " ...type < ..integer\n ...family < .other\n ...out > ...alias\n", result_target_source);
    const StaticResultContext *context = publication_context(result_applied_environment());
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_field("field"), &publication));
    assert(publication->value == context->creation->vertex->value);
    assert(publication->first_output->value == publication->value);
    assert(publication->value->as.nominal.first_capture->next->value == target_constructor);
    assert(target_constructor->as.nominal.residual_count == 1 && guards.status == STATIC_GUARD_OK);
    destroy_publications();
}

/** Even a closed target application cannot compare itself with an unknown
 * ancestor. The guard's Pending must propagate through a known application;
 * independent formal evidence then permits retry of that same source use.
 */
static void it_propagates_pending_ancestry_until_independent_keys_close(void) {
    setup_publications("@<\n...slot\n@>\n...out\n...slot\n@!\nanchor&\n..integer\n"
        "field&\n.main\n ...slot < ..integer\n ...out > ...alias\n", "@!\n");
    const StaticResultContext *context = publication_context(caller_environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate(&results, context, result_field("field"), &outcome));
    assert(outcome == STATIC_RESULT_PENDING && guards.status == STATIC_GUARD_OK);
    StaticResultUse *use;
    assert(static_result_use(&results, context, result_field("field"), NULL, &use));
    assert(use->producer == NULL && results.definition_use_count == 0);
    for (const StaticGuardOwner *owner = guards.first_owner; owner; owner = owner->next) {
        if (owner->environment->definition == caller_module) { assert(owner == context->creation->owner); }
    }
    size_t paths = guards.path_count, contexts = results.context_count;
    assert(static_result_evaluate(&results, context, result_field("field"), &outcome));
    assert(outcome == STATIC_RESULT_PENDING && guards.path_count == paths && results.context_count == contexts);
    assert(use->producer == NULL && results.definition_use_count == 0);
    const StaticTerm *slot;
    assert(static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &slot));
    assert(static_constraint_equate(&solver, slot, integer_type, (SourceSpan){0}));
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, result_field("field"), &publication));
    assert(publication->first_output->value == integer_type && use->producer != NULL);
    assert(use->producer->creation->vertex == context->creation->vertex);
    destroy_publications();
}

/** Prime only B's independently selected output, without an A ancestor. */
static void warm_target_output(void) {
    const StaticTermEnvironment *environment;
    assert(static_term_environment(&terms, target_module, target_constructor, &environment));
    const StaticResultContext *independent = publication_context(environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, independent,
        result_named(target_module, "...out"), &outcome) && outcome == STATIC_RESULT_READY);
    assert(results.cache_count > 0);
}

/** An independently cached B result returns A(Integer). Under a new unknown
 * A owner, B's old proof cannot bypass the deferred A-key comparison. Cold and
 * warm cases must both wait, then agree on equal or unequal independent evidence.
 */
static void it_revalidates_cached_result_proofs_under_unknown_incoming_ancestry(void) {
    for (size_t warm = 0; warm < 2; warm++) {
        for (size_t unequal = 0; unequal < 2; unequal++) {
            setup_publications("@<\n...slot\n@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...alias\n",
                "@>\n...out\n.main\n ...slot < ..integer\n@!\n");
            if (warm) { warm_target_output(); }
            const StaticResultContext *context = publication_context(caller_environment);
            StaticResultOutcome outcome;
            assert(static_result_evaluate(&results, context, result_field("field"), &outcome));
            assert(outcome == STATIC_RESULT_PENDING && guards.status == STATIC_GUARD_OK);
            assert(publications.count == 0 && publications.alias_count == 0);
            const StaticTerm *slot;
            assert(static_term_symbol(&terms, caller_environment, application_formal(caller_module, 0), &slot));
            assert(static_constraint_equate(&solver, slot, unequal ? target_constructor : integer_type, (SourceSpan){0}));
            const StaticPublication *output = NULL;
            bool success = static_publication_resolve(&publications, context, result_field("field"), &output);
            assert(success == !unequal);
            if (!unequal) { assert(output->first_output->value == context->creation->vertex->value); }
            else {
                assert(output == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
                assert(guards.rejected_creation_value != NULL && guards.rejected_creation_value != guards.earlier_creation_value);
            }
            destroy_publications();
        }
    }
}

/** An unrelated unknown A parameter must not prevent B's constant result.
 * The foreign proof is replayed once in the actual context and then reused
 * there without demanding global root closure or manufacturing more paths.
 */
static void it_reuses_actual_context_proofs_without_requiring_unrelated_ancestor_keys(void) {
    setup_publications("@<\n...slot\n@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...alias\n",
        "@>\n...out\n..integer\n@!\n");
    warm_target_output();
    const StaticResultContext *context = publication_context(caller_environment);
    const StaticPublication *output, *again;
    assert(static_publication_resolve(&publications, context, result_field("field"), &output));
    assert(output->first_output->value == integer_type && context->creation->vertex == NULL);
    StaticResultUse *source;
    assert(static_result_use(&results, context, result_field("field"), NULL, &source));
    StaticResultDefinitionUse *definition;
    assert(static_result_definition_use(&results, source->producer, result_named(target_module, "...out"),
        NULL, result_field("field")->span, &definition));
    bool hit = false;
    assert(static_result_cache_read(&results, definition, &hit) && hit);
    assert(definition->checked_rhs->context == source->producer);
    size_t paths = guards.path_count, contexts = results.context_count;
    assert(static_publication_resolve(&publications, context, result_field("field"), &again) && again == output);
    assert(guards.path_count == paths && results.context_count == contexts);
    destroy_publications();
}

/** Refuse every actual allocation when a foreign checked value must replay its
 * proof under an unknown incoming ancestor. Previous cache records remain owned;
 * failures never publish an outcome or leak partially evaluated source work.
 */
static void it_unwinds_every_foreign_cache_proof_replay_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_publications("@<\n...slot\n@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...alias\n",
            "@>\n...out\n.main\n ...slot < ..integer\n@!\n");
        warm_target_output();
        const StaticResultContext *context = publication_context(caller_environment);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        bool success = static_result_evaluate(&results, context, result_field("field"), &outcome);
        if (!cut) { assert(success && outcome == STATIC_RESULT_PENDING); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && outcome == STATIC_RESULT_READY && results.status == STATIC_RESULT_OUT_OF_MEMORY); }
        failure_at = 0;
        assert(publications.count == 0 && publications.alias_count == 0);
        destroy_publications();
    }
}

/** Closing a nominal producer does not close a definition that projects its
 * own result, directly or through the other module. Retain the value-cycle
 * witness rather than blaming a permitted same-key nominal dependency.
 */
static void it_rejects_direct_and_mutual_static_projection_cycles(void) {
    const char *callers[] = {
        "@<\n...slot\n@>\n...out\n.main\n ...slot < ...slot\n ...out > ...out\n"
            "@!\nanchor&\n..integer\nfield&\n.main\n ...slot < ..integer\n ...out > ...alias\n",
        "@<\n...slot\n@>\n...out\n.other\n ...slot < ...slot\n ...out > ...out\n"
            "@!\nanchor&\n..integer\nfield&\n.main\n ...slot < ..integer\n ...out > ...alias\n"
    };
    const char *other = "@<\n...slot\n@>\n...out\n.main\n ...slot < ...slot\n ...out > ...out\n@!\n";
    for (size_t mode = 0; mode < 2; mode++) {
        setup_publications(callers[mode], mode ? other : "@!\n");
        const StaticResultContext *context = closed_context();
        const StaticPublication *output = NULL;
        assert(!static_publication_resolve(&publications, context, result_field("field"), &output));
        assert(output == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
        assert(guards.repeated_definition_ancestor != NULL && guards.rejected_definition != NULL);
        assert(guards.creation_witness == NULL && guards.rejected_creation_value == NULL);
        assert(diagnostic_valid(&publications.error));
        destroy_publications();
    }
}

/** Sweep every allocation through the real recursive result/publication
 * composition, retaining untouched public output and counted full teardown.
 */
static void it_unwinds_every_nominal_self_result_publication_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup_publications(self_source, "@!\n");
        const StaticResultContext *context = closed_context();
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        const StaticPublication *output = NULL;
        bool success = static_publication_resolve(&publications, context, result_field("field"), &output);
        if (!cut) { assert(success && output); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && output == NULL && publications.status == STATIC_PUBLICATION_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_publications();
    }
}

/** Execute nominal/pending/value-cycle and resource-boundary compositions. */
static void run(void) {
    it_publishes_the_producers_own_nominal_type_without_value_recursion();
    it_preserves_closed_recursive_identity_with_constructor_captures();
    it_propagates_pending_ancestry_until_independent_keys_close();
    it_revalidates_cached_result_proofs_under_unknown_incoming_ancestry();
    it_reuses_actual_context_proofs_without_requiring_unrelated_ancestor_keys();
    it_unwinds_every_foreign_cache_proof_replay_allocation_refusal();
    it_rejects_direct_and_mutual_static_projection_cycles();
    it_unwinds_every_nominal_self_result_publication_allocation_refusal();
}
/** Return success only after every assertion and counted cleanup completes. */
int main(void) { run(); return 0; }
