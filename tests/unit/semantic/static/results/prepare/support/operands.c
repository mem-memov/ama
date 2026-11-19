/**
 * @file
 * @brief Verify private operand preparation through live expressions with current ordinary syntax.
 *
 * Real same-Type reference edges supply distinct incoming contexts. Cached
 * metadata must not merge their provenance or certify either actual use.
 */

/** All fields use ordinary concrete Type lines; self-reference adds no eager ownership cycle. */
static const char *operand_caller_source =
    "@!\nanchor&\n..integer\nnext&\n.main\nvalue&\n..integer\n";

/** Own real source/term/guard dependencies before a fresh result plan. */
static void setup_operand_sources(void) {
    setup_guards(operand_caller_source, "@!\n");
    initialize_results();
}

/** Enter one explicitly named same-Type dependency through the actual guarded application operation. */
static const StaticResultContext *operand_child_context(const StaticResultContext *parent) {
    StaticResultUse *next;
    bool registered = static_result_use(&results, parent, result_field("next"), NULL, &next);
    assert(registered);
    StaticApplicationOutcome outcome;
    bool applied = static_result_apply(&results, next, &outcome);
    assert(applied && outcome == STATIC_APPLY_RESOLVED && next->producer != NULL);
    return next->producer;
}

/** Same source/environment keys share immutable operands despite distinct incoming contexts.
 * The generic and supplied nominal environments remain separate even for one source AST.
 */
static void it_shares_only_same_source_environment_operands_across_actual_contexts(void) {
    setup_operand_sources();
    StaticResultUse *root = prepared_root("value");
    const StaticResultContext *first_context = operand_child_context(root->context);
    const StaticResultContext *second_context = operand_child_context(first_context);
    assert(first_context != second_context && first_context->creation != second_context->creation);
    assert(first_context->creation->owner->environment == second_context->creation->owner->environment);
    StaticResultUse *first, *second;
    bool registered = static_result_use(&results, first_context, result_field("value"), NULL, &first);
    assert(registered);
    registered = static_result_use(&results, second_context, result_field("value"), NULL, &second);
    assert(registered && first != second && first->source == second->source && root->source != first->source);
    bool prepared = static_result_prepare(&results, root);
    assert(prepared);
    prepared = static_result_prepare(&results, first);
    assert(prepared && root->prepared != first->prepared);
    size_t count = results.operand_count, allocations_before = allocation_count;
    size_t paths = guards.path_count, equations = solver.equation_count;
    failure_at = allocation_count + 1;
    prepared = static_result_prepare(&results, second);
    failure_at = 0;
    assert(prepared && second->prepared == first->prepared && allocation_count == allocations_before);
    assert(results.operand_count == count && guards.path_count == paths && solver.equation_count == equations);
    assert(!first->closed && !second->closed && first->producer == NULL && second->producer == NULL);
    assert(first->outcome == STATIC_RESULT_PENDING && second->outcome == STATIC_RESULT_PENDING);
    destroy_results();
}

/** Invalid source identity/output and count overflow preserve published preparation and caller output. */
static void it_refuses_foreign_operand_sources_receivers_and_cache_count_overflow(void) {
    for (unsigned kind = 0; kind < 4; kind++) {
        setup_operand_sources();
        StaticResultUse *use = prepared_root("value");
        bool prepared = static_result_prepare(&results, use);
        assert(prepared);
        StaticResultUse *next;
        bool registered = static_result_use(&results, use->context, result_field("next"), NULL, &next);
        assert(registered);
        StaticResultSource copied = *next->source;
        const StaticTermApplication *output = use->prepared;
        StaticResultOperands *published = results.first_operands;
        size_t count = results.operand_count, before = allocation_count;
        if (kind == 3) { results.operand_count = SIZE_MAX; }
        prepared = static_result_operands(&results,
            kind == 0 ? &copied : kind == 1 ? NULL : next->source, kind == 2 ? NULL : &output);
        assert(!prepared && output == use->prepared && results.first_operands == published && allocation_count == before);
        assert(results.operand_count == (kind == 3 ? SIZE_MAX : count));
        assert(results.status == (kind == 3 ? STATIC_RESULT_OUT_OF_MEMORY : STATIC_RESULT_INVALID));
        results.operand_count = count;
        destroy_results();
    }
}

/** Every measured live-consumer allocation refuses without publishing a cache entry or prepared use. */
static void it_unwinds_every_live_zero_input_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_operand_sources();
        StaticResultUse *use = prepared_root("value");
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool prepared = static_result_prepare(&results, use);
        if (!failure) {
            assert(prepared && use->prepared != NULL && results.operand_count == 1);
            total = allocation_count - before;
            assert(total != 0);
        } else {
            assert(!prepared && use->prepared == NULL && results.operand_count == 0 && results.first_operands == NULL);
            assert(results.status == STATIC_RESULT_OUT_OF_MEMORY && diagnostic_valid(&results.error));
        }
        failure_at = 0;
        destroy_results();
    }
}
