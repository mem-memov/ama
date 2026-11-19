/** @file
 * @brief Verify current graph values, anchored omitted inputs, exact alias reads and true definition-cycle refusal.
 */
#include "../common.c"

/** Build only actual graph syntax; there are no ordinary static sections. */
static void setup_values(const char *caller, const char *target) {
    setup_guards(caller, target);
    initialize_results();
}

/** Register an explicitly supplied compilation context before evaluating values. */
static const StaticResultContext *value_context(const StaticTermEnvironment *environment) {
    const StaticCreationPath *path;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, environment, &path));
    assert(static_result_context(&results, path, &context));
    return context;
}

/** Inspect actual canonical metadata through the public finite solver query. */
static const StaticTerm *defined_value(const StaticTermEnvironment *environment, const char *name) {
    const StaticSymbol *symbol = result_named(environment->definition, name);
    const StaticTerm *term, *value;
    StaticConstraintOutcome outcome;
    assert(static_term_symbol(&terms, environment, symbol, &term));
    assert(static_constraint_value(&solver, term, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE);
    return value;
}

/** A pair denotes the ordinary value, while its owner is a compiler-only
 * producer map. Checked metadata does not complete ordinary body admission.
 */
static void it_evaluates_concrete_pairs_and_caches_by_complete_producer_owner_keys(void) {
    setup_values("@...\n...out\n..integer\n", "@...\n...out\n..integer\n");
    const StaticResultContext *context = value_context(caller_environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, context, result_named(caller_module, "...out"), &outcome));
    assert(outcome == STATIC_RESULT_READY && defined_value(caller_environment, "...out") == integer_type);
    assert(context->creation->owner->value == NULL && results.cache_count == 1);
    assert(results.first_cache->owner == context->registry_use->candidate->key);
    assert(results.first_cache->value == integer_type && results.first_graph->context == context);
    assert(!context->registry_use->candidate->local_complete);
    destroy_results();
}

/** An existing pair supplies real equality evidence for an omitted foreign
 * input. The receipt does not redefine that pair or add a definition edge.
 */
static void it_infers_omitted_inputs_from_existing_receiver_definitions_without_false_data_cycles(void) {
    setup_values("@...\n...known\n..integer\n.other\n ...out > ...known\n", "@...\n...out\n...type\n");
    const StaticResultContext *context = value_context(caller_environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, context, result_named(caller_module, "...known"), &outcome));
    assert(outcome == STATIC_RESULT_READY);
    assert(static_result_evaluate(&results, context, result_projection(0), &outcome));
    StaticResultUse *use;
    assert(static_result_use(&results, context, result_projection(0), NULL, &use));
    assert(use->source->value == NULL && use->producer_request->input_count == 1);
    const StaticTerm *input;
    StaticConstraintOutcome closed;
    /* The receiver binds the public result first; its defining forwarding
     * equation is checked independently rather than guessed from that receipt. */
    assert(static_result_evaluate_definition(&results, use->producer,
        result_named(target_module, "...out"), &outcome));
    assert(static_constraint_value(&solver, use->producer_request->inputs[0].value, &input, &closed));
    assert(closed == STATIC_CONSTRAINT_COMPLETE && input == integer_type);
    assert(static_result_evaluate(&results, context, result_projection(0), &outcome));
    assert(outcome == STATIC_RESULT_READY && use->closed);
    assert(defined_value(caller_environment, "...known") == integer_type);
    assert(guards.definition_edge_count == 0 && caller_module->parameter_count == 0);
    destroy_results();
}

/** Equal-key creation cycles remain allowed, but A.alias -> B.out -> A.alias
 * is a true value-definition cycle and must fail before unbounded evaluation.
 */
static void it_rejects_real_selected_alias_cycles_even_when_creation_keys_are_equal(void) {
    setup_values("@...\n.other\n ...out > ...alias\n...anchor\n..integer\n",
        "@...\n.main\n ...alias > ...out\n");
    const StaticResultContext *context = value_context(caller_environment);
    StaticResultOutcome outcome = STATIC_RESULT_PENDING;
    assert(!static_result_evaluate_definition(&results, context, result_named(caller_module, "...alias"), &outcome));
    assert(outcome == STATIC_RESULT_PENDING && results.status == STATIC_RESULT_CONFLICT);
    assert(guards.status == STATIC_GUARD_CONFLICT && guards.repeated_definition_ancestor);
    assert(guards.creation_witness == NULL && results.error.message == guards.error.message);
    assert(diagnostic_valid(&results.error));
    destroy_results();
}

/** Type and partially applied constructor inputs are values; forwarding a
 * Template preserves its original full residual signature and captures.
 */
static void it_forwards_supplied_type_and_template_metadata_without_rebinding_their_origins(void) {
    setup_results();
    const StaticTermEnvironment *environment = result_applied_environment();
    const StaticResultContext *context = value_context(environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, context, result_named(caller_module, "...first"), &outcome));
    assert(outcome == STATIC_RESULT_READY && defined_value(environment, "...first") == integer_type);
    assert(static_result_evaluate_definition(&results, context, result_named(caller_module, "...partial"), &outcome));
    assert(outcome == STATIC_RESULT_READY && defined_value(environment, "...partial") == result_family_constructor);
    assert(result_family_constructor->category == STATIC_CATEGORY_TEMPLATE);
    assert(result_family_constructor->as.nominal.residual_count == 1);
    assert(caller_module->parameter_count == 2);
    destroy_results();
}

/** A received parameter is metadata by value even when represented by another
 * module's derived term. Its known value is supplied input IR; the callee must
 * not search for a foreign context or invent a definition-read edge.
 */
static void it_forwards_derived_argument_metadata_without_reading_a_foreign_definition(void) {
    setup_values("@...\n...provided\n..integer\n", "@...\n...out\n...type\n");
    const StaticTerm *provided;
    assert(static_term_symbol(&terms, caller_environment, result_named(caller_module, "...provided"), &provided));
    assert(provided->as.variable.role == STATIC_VARIABLE_DERIVED);
    /* This exact fact agrees with the supplying pair's ordinary integer Type;
     * it is argument evidence, not evaluation of that pair inside the callee. */
    assert(static_constraint_equate(&solver, provided, integer_type, provided->site));
    StaticTermCapture input = {.formal = application_formal(target_module, 0), .value = provided};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, target_module, &input, 1, &environment));
    const StaticResultContext *context = value_context(environment);
    StaticResultOutcome outcome;
    assert(static_result_evaluate_definition(&results, context, result_named(target_module, "...out"), &outcome));
    assert(outcome == STATIC_RESULT_READY && defined_value(environment, "...out") == integer_type);
    assert(guards.definition_edge_count == 0 && results.context_count == 1);
    destroy_results();
}

/** Run current source graph value and exact dependency partitions. */
static void run(void) {
    it_forwards_derived_argument_metadata_without_reading_a_foreign_definition();
    it_evaluates_concrete_pairs_and_caches_by_complete_producer_owner_keys();
    it_infers_omitted_inputs_from_existing_receiver_definitions_without_false_data_cycles();
    it_rejects_real_selected_alias_cycles_even_when_creation_keys_are_equal();
    it_forwards_supplied_type_and_template_metadata_without_rebinding_their_origins();
}
/** Return only after counted ownership teardown. */
int main(void) { run(); return 0; }
