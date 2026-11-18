/** @file
 * @brief Verify iterative static output equations against independently expected nominal values and guards.
 */
#include "../common.c"

/** Register the exact caller as an explicit compilation context. */
static const StaticResultContext *root_context(const StaticTermEnvironment *environment) {
    const StaticCreationPath *root;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, environment, &root));
    assert(static_result_context(&results, root, &context));
    return context;
}

/** Report a real structural failure with its original compiler diagnostic. */
static bool evaluate(const StaticResultContext *context, const AstTypeExpression *expression,
    StaticResultOutcome *outcome) {
    bool success = static_result_evaluate(&results, context, expression, outcome);
    if (!success) { fprintf(stderr, "Static evaluation failure: %s\n", results.error.message); }
    return success;
}

/** Nested source inputs retain caller scope, while actual output fields bind
 * producer scope and aliases retain selected values rather than producer Types.
 */
static void it_evaluates_nested_inputs_and_actual_selected_output_values(void) {
    setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ...slot < ..integer\n ...out > ...alias\nsame&\n...alias\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_READY);
    const StaticTerm *alias, *value;
    StaticConstraintOutcome solved;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...alias"), &alias));
    assert(static_constraint_value(&solver, alias, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(evaluate(context, result_field("same"), &outcome) && outcome == STATIC_RESULT_READY);
    assert(results.cache_count >= 2 && caller_module->parameter_count == 0);
    destroy_results();
}

/** A returned family retains its own local residual, and later source inputs
 * instantiate it through the same ordinary structural application machinery.
 */
static void it_returns_partial_families_and_uses_them_as_actual_heads(void) {
    setup_guards("@!\nanchor&\n..integer\nfactory&\n.other\n ...slot < ..integer\n ...family > ...family alias\nfield&\n...family alias\n ... < ..integer\n", "@<\n...slot\n@>\n...family\n.other\n@!\n");
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("factory"), &outcome) && outcome == STATIC_RESULT_READY);
    const StaticTerm *alias, *value;
    StaticConstraintOutcome solved;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...family alias"), &alias));
    assert(static_constraint_value(&solver, alias, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == target_constructor && value->as.nominal.residual_count == 1);
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_READY);
    StaticResultUse *field;
    assert(static_result_use(&results, context, result_field("field"), NULL, &field));
    assert(static_constraint_value(&solver, field->source->value, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value->as.nominal.residual_count == 0);
    assert(value->as.nominal.first_capture->value == integer_type);
    destroy_results();
}

/** Independent captures distinguish generic and applied definition environments
 * without requiring unrelated root closure for selected result equations.
 */
static void it_reads_scoped_results_in_applied_environments(void) {
    setup_results();
    const StaticTermEnvironment *applied = result_applied_environment();
    const StaticResultContext *context = root_context(applied);
    const StaticSymbol *first = result_named(caller_module, "...first");
    const StaticResultDefinition *definition;
    assert(static_result_definition(&results, applied, first, &definition));
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, context, first, &outcome) && outcome == STATIC_RESULT_READY);
    const StaticTerm *value;
    StaticConstraintOutcome solved;
    assert(static_constraint_value(&solver, definition->value, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    destroy_results();
}

/** A skipped selected output does not excuse a different invalid output's
 * definition. Producer validation includes the entire declared output set.
 */
static void it_checks_unselected_output_definition_cycles_before_returning(void) {
    setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ... >\n", "@>\n...good\n..integer\n...bad\n...bad\n@!\n");
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome = STATIC_RESULT_READY;
    assert(!static_result_evaluate(&results, context, result_field("field"), &outcome));
    assert(outcome == STATIC_RESULT_READY && results.status == STATIC_RESULT_CONFLICT);
    assert(guards.repeated_definition_ancestor != NULL && results.definition_use_count >= 2);
    destroy_results();
}

/** Unknown actual origins remain equations; no output name chooses a module. */
static void it_retains_pending_unknown_producer_origins_without_guessing(void) {
    setup_guards("@<\n...family\n ...slot\n@!\nanchor&\n..integer\nfield&\n...family\n ...slot < ..integer\n ...out > ...alias\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_PENDING);
    assert(results.definition_use_count == 0 && guards.path_count == 2);
    destroy_results();
}

/** An existing explicit result receiver is an independent equality anchor,
 * not a read of its own value definition through a fictitious dependency.
 */
static void it_equates_existing_result_receivers_without_definition_self_cycles(void) {
    setup_guards("@>\n...expected\n..integer\n@!\nanchor&\n..integer\nfield&\n.other\n ...slot < ..integer\n ...out > ...expected\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_READY);
    assert(guards.repeated_definition_ancestor == NULL);
    const StaticTerm *expected, *value;
    StaticConstraintOutcome solved;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...expected"), &expected));
    assert(static_constraint_value(&solver, expected, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    destroy_results();
}

/** Producer inference remains open even when the selected output is constant;
 * selected equations do not authorize publishing a complete producer cache.
 */
static void it_keeps_constant_results_uncached_for_unresolved_producer_inputs(void) {
    setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...alias\n", "@<\n...unused\n@>\n...out\n..integer\n@!\n");
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_READY);
    StaticResultUse *use;
    assert(static_result_use(&results, context, result_field("field"), NULL, &use));
    assert(!use->closed && use->producer != NULL && results.cache_count == 0);
    const StaticTerm *alias, *value;
    StaticConstraintOutcome solved;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...alias"), &alias));
    assert(static_constraint_value(&solver, alias, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    destroy_results();
}

/** Exact original template ancestry rejects a source repeat before its output
 * expression can expand, including a complete result already in the value cache.
 */
static void it_checks_creation_ancestry_before_expanding_producer_definitions(void) {
    setup_guards("@<\n...slot\n@>\n...out\n..integer\n@!\nanchor&\n..integer\nfield&\n.main\n ... >\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome = STATIC_RESULT_READY;
    assert(!static_result_evaluate(&results, context, result_field("field"), &outcome));
    assert(outcome == STATIC_RESULT_READY && guards.repeated_ancestor == context->creation);
    assert(results.definition_use_count == 0);
    destroy_results();
}

/** Sweep every allocation in a real output evaluation, including continuations,
 * solver work, application normalization, both guard graphs and cache storage.
 * No failed attempt publishes an outcome; all owners unwind to zero allocations.
 */
static void it_unwinds_every_iterative_output_evaluation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ...slot < ..integer\n ...out > ...alias\n", result_target_source);
        static_result_init(&results, &guards, sources.allocator);
        const StaticResultContext *context = root_context(caller_environment);
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool success = static_result_evaluate(&results, context, result_field("field"), &outcome);
        if (!failure) { assert(success && outcome == STATIC_RESULT_READY); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && outcome == STATIC_RESULT_PENDING && results.status == STATIC_RESULT_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_results();
    }
}

/** Positional skips advance only the output sequence. Every producer output
 * is checked, while the expression value remains its whole producer Type.
 */
static void it_preserves_positional_output_skips_and_whole_producer_types(void) {
    setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ... >\n ... > ...selected\n", "@>\n...first\n..integer\n...second\n..integer\n@!\n");
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_READY);
    StaticResultUse *use;
    assert(static_result_use(&results, context, result_field("field"), NULL, &use));
    const StaticTerm *value, *selected;
    StaticConstraintOutcome solved;
    assert(static_constraint_value(&solver, use->source->value, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value->as.nominal.origin == target_module);
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...selected"), &selected));
    assert(static_constraint_value(&solver, selected, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(results.definition_use_count == 2);
    destroy_results();
}

/** Actual static interfaces reject runtime fields, absent names and too many
 * output positions before any body/default/native behavior can be reached.
 */
static void it_rejects_absent_runtime_and_over_arity_static_output_requests(void) {
    const char *callers[] = {
        "@!\nanchor&\n..integer\nfield&\n.other\n ...missing > ...alias\n",
        "@!\nanchor&\n..integer\nfield&\n.other\n ...runtime > ...alias\n",
        "@!\nanchor&\n..integer\nfield&\n.other\n ... >\n ... >\n"
    };
    for (size_t index = 0; index < 3; index++) {
        setup_guards(callers[index], "@>\n...out\n..integer\nruntime\n..integer\n@!\n");
        static_result_init(&results, &guards, sources.allocator);
        const StaticResultContext *context = root_context(caller_environment);
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        assert(!static_result_evaluate(&results, context, result_field("field"), &outcome));
        assert(outcome == STATIC_RESULT_READY && results.status == STATIC_RESULT_CONFLICT);
        assert(diagnostic_valid(&results.error) && results.definition_use_count == 0);
        destroy_results();
    }
}

/** Public invalid receivers and copied contexts fail before publishing work;
 * an unchanged Pending retry retains semantic records rather than new frames.
 */
static void it_rejects_invalid_evaluation_handles_and_preserves_pending_work_identity(void) {
    for (size_t mode = 0; mode < 3; mode++) {
        setup_guards("@<\n...family\n ...slot\n@!\nanchor&\n..integer\nfield&\n...family\n ...slot < ..integer\n ...out > ...alias\n", result_target_source);
        static_result_init(&results, &guards, sources.allocator);
        const StaticResultContext *context = root_context(caller_environment);
        StaticResultContext copied = *context;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        if (mode < 2) {
            assert(!static_result_evaluate(&results, mode == 0 ? context : &copied,
                result_field("field"), mode == 0 ? NULL : &outcome));
            assert(results.status == STATIC_RESULT_INVALID && results.use_count == 0 && outcome == STATIC_RESULT_READY);
        } else {
            assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_PENDING);
            size_t applications_before = applications.count, uses_before = results.use_count, paths_before = guards.path_count;
            assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_PENDING);
            assert(applications.count == applications_before && results.use_count == uses_before && guards.path_count == paths_before);
        }
        destroy_results();
    }
}

/** Finite written nesting repeats one template in two sibling creation edges,
 * not a forbidden producer-to-itself edge. The selected value is the inner Type.
 */
static void it_attributes_finite_repeated_constructor_inputs_to_the_common_lexical_owner(void) {
    setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ...slot < .other\n  ...slot < ..integer\n ...out > ...alias\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    const StaticResultContext *context = root_context(caller_environment);
    StaticResultOutcome outcome;
    assert(evaluate(context, result_field("field"), &outcome) && outcome == STATIC_RESULT_READY);
    const StaticTerm *alias, *value;
    StaticConstraintOutcome solved;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...alias"), &alias));
    assert(static_constraint_value(&solver, alias, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value->as.nominal.origin == target_module);
    assert(value->as.nominal.first_capture->value == integer_type);
    size_t siblings = 0;
    for (const StaticCreationPath *path = guards.first_path; path; path = path->next) {
        if (path->owner->environment->definition == target_module) {
            assert(path->parent == context->creation);
            siblings++;
        }
    }
    assert(siblings == 2 && guards.repeated_ancestor == NULL);
    destroy_results();
}

/** Execute structural results, applied scope, all-output validation and Pending. */
static void run(void) {
    it_evaluates_nested_inputs_and_actual_selected_output_values();
    it_returns_partial_families_and_uses_them_as_actual_heads();
    it_reads_scoped_results_in_applied_environments();
    it_checks_unselected_output_definition_cycles_before_returning();
    it_retains_pending_unknown_producer_origins_without_guessing();
    it_equates_existing_result_receivers_without_definition_self_cycles();
    it_keeps_constant_results_uncached_for_unresolved_producer_inputs();
    it_checks_creation_ancestry_before_expanding_producer_definitions();
    it_unwinds_every_iterative_output_evaluation_allocation_refusal();
    it_preserves_positional_output_skips_and_whole_producer_types();
    it_rejects_absent_runtime_and_over_arity_static_output_requests();
    it_rejects_invalid_evaluation_handles_and_preserves_pending_work_identity();
    it_attributes_finite_repeated_constructor_inputs_to_the_common_lexical_owner();
}
/** Return success after counted owner teardown. */
int main(void) { run(); return 0; }
