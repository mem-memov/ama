/** @file
 * @brief Verify exact declaration result selection and complete-supplier inference boundaries.
 */
#include "support/prepare.c"
#include "semantic/static/publication/plan.h"

/** A selector names an original foreign public result and gets a scalar
 * receiver, while its producing module has no invented nominal Type.
 */
static void it_reads_one_exact_foreign_result_after_named_input_closure(void) {
    const StaticResultContext *context = setup_selection(selected_caller, "@...\n.out\n.slot\n");
    const AstTypeExpression *expression = result_field("data");
    const StaticResultSource *source;
    assert(static_result_source(&results, caller_environment, expression, &source));
    assert(source->producer == target_module && source->head == NULL && source->value);
    assert(source->selection == result_named(target_module, ".out"));
    assert(solver.equation_count == 0 && target_constructor == NULL);
    StaticResultUse *use;
    assert(evaluate_selection(context, expression, &use));
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, source->value, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type && use->closed);
    assert(use->selection && use->selection->definition->symbol == source->selection);
    assert(use->producer->creation->parent == context->creation && use->required_graph->local_complete);
    size_t sources_before = results.source_count, paths_before = guards.path_count;
    assert(evaluate_selection(context, expression, &use));
    assert(results.source_count == sources_before && guards.path_count == paths_before);
    destroy_results();
}

/** Even a known selected output cannot erase an unused incomplete supplier
 * input. Consumer expectations remain disconnected until independent evidence.
 */
static void it_keeps_expected_scalar_types_disconnected_from_unresolved_supplier_inputs(void) {
    for (size_t dependent = 0; dependent < 2; dependent++) {
        const StaticResultContext *context = setup_selection(
            "@!\ndata\n/test/fixture/v1.0.0/src/other\n .out\nanchor&\n//integer\n",
            dependent ? "@...\n.out\n.slot\n" : "@...\n.out\n//integer\n.other\n.slot\n");
        const AstTypeExpression *expression = result_field("data");
        StaticResultUse *use;
        assert(static_result_use(&results, context, expression, NULL, &use));
        assert(static_constraint_equate(&solver, use->source->value, integer_type, expression->head));
        assert(evaluate_selection(context, expression, &use));
        assert(!use->closed && use->producer_request->input_count == 1);
        const StaticTerm *hole = use->producer_request->inputs[0].value, *value;
        StaticConstraintOutcome outcome;
        assert(static_constraint_value(&solver, hole, &value, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
        assert(!use->required_graph->local_complete);
        assert(static_constraint_equate(&solver, hole, integer_type, expression->head));
        assert(evaluate_selection(context, expression, &use) && use->closed && use->required_graph->local_complete);
        destroy_results();
    }
}

/** Selected operands use the common lexical parent, not the outer application
 * result. The producer requirement survives a scalar result and warm reuse.
 */
static void it_evaluates_nested_selections_under_the_original_lexical_owner(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/family", "@<\nvalue\n.slot\n@!\n");
    const StaticResultContext *context = setup_selection(
        "@!\ndata\n/test/fixture/v1.0.0/src/family\n .slot < /test/fixture/v1.0.0/src/other\n  .slot < //integer\n  .out\nanchor&\n//integer\n",
        "@...\n.out\n.slot\n");
    const AstTypeExpression *expression = result_field("data");
    StaticResultUse *outer;
    assert(evaluate_selection(context, expression, &outer) && outer->closed);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, outer->source->value, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value->category == STATIC_CATEGORY_TYPE);
    assert(value->as.nominal.first_capture->value == integer_type);
    StaticResultUse *inner;
    assert(static_result_use(&results, context, expression->first->input, NULL, &inner));
    assert(inner->closed && inner->selection && inner->producer->creation->parent == context->creation);
    assert(outer->producer->creation->parent == context->creation);
    destroy_results();
}

/** A selected static result may be a complete residual Template; ordinary
 * storage admission, rather than scalar evaluation, owns its later restriction.
 */
static void it_preserves_selected_partial_constructor_identity_without_runtime_admission(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/family", "@<\nvalue\n.slot\n@!\n");
    const StaticResultContext *context = setup_selection(selected_caller,
        "@...\n.out\n/test/fixture/v1.0.0/src/family\n.other\n.slot\n");
    StaticResultUse *use;
    assert(evaluate_selection(context, result_field("data"), &use) && use->closed);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, use->source->value, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value->category == STATIC_CATEGORY_TEMPLATE);
    assert(value->as.nominal.origin->parameter_count == 1 && value->as.nominal.first_residual);
    assert(use->required_graph->local_complete && use->source->selection == result_named(target_module, ".out"));
    destroy_results();
}

/** Foreign input names and absent exports cannot be selected. Source binding
 * refuses before inventing a scalar value or publishing a source record.
 */
static void it_refuses_nonpublic_or_missing_foreign_selections_without_publishing_sources(void) {
    const char *callers[] = {
        "@!\ndata\n/test/fixture/v1.0.0/src/other\n .slot\nanchor&\n//integer\n",
        "@!\ndata\n/test/fixture/v1.0.0/src/other\n .absent\nanchor&\n//integer\n"
    };
    for (size_t index = 0; index < sizeof(callers) / sizeof(*callers); index++) {
        (void)setup_selection(callers[index], "@...\n.out\n.slot\n");
        const StaticResultSource *source = NULL;
        size_t before = allocation_count;
        assert(!static_result_source(&results, caller_environment, result_field("data"), &source));
        assert(source == NULL && results.source_count == 0 && results.status == STATIC_RESULT_INVALID);
        assert(allocation_count == before && solver.equation_count == 0);
        destroy_results();
    }
}

/** Alias exports retain their actual defining producer and definition paths;
 * selecting an alias does not invent an ordinary Type for either supplier.
 */
static void it_reads_alias_exports_with_checked_foreign_definition_provenance(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/provider", "@...\n.value\n//integer\n");
    const StaticResultContext *context = setup_selection(selected_caller,
        "@...\n/test/fixture/v1.0.0/src/provider\n .value > .out\n.other\n.slot\n");
    StaticResultUse *use;
    assert(evaluate_selection(context, result_field("data"), &use) && use->closed);
    assert(use->source->selection->role == STATIC_ALIAS && use->selection->selected);
    const StaticResultDefinitionUse *selected = use->selection->selected;
    assert(selected->definition->symbol != use->source->selection);
    assert(selected->context->creation->parent == use->producer->creation);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, use->source->value, &value, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    destroy_results();
}

/** Scalar selection never suppresses validation of an unrelated public result,
 * and a fully determined foreign Type must still match consumer expectations.
 */
static void it_rejects_invalid_unselected_outputs_and_closed_consumer_type_conflicts(void) {
    for (size_t conflict = 0; conflict < 2; conflict++) {
        const StaticResultContext *context = setup_selection(selected_caller, conflict ?
            "@...\n.out\n//text\n.other\n.slot\n" :
            "@...\n.out\n.slot\n.bad\n.bad\n");
        StaticResultUse *use;
        const AstTypeExpression *expression = result_field("data");
        assert(static_result_use(&results, context, expression, NULL, &use));
        if (conflict) { assert(static_constraint_equate(&solver, use->source->value, integer_type, expression->head)); }
        assert(!evaluate_selection(context, expression, &use) && results.status != STATIC_RESULT_OK);
        destroy_results();
    }
}

/** Every measured allocation failure leaves the sticky failed plan owned for
 * cleanup and never advertises a successfully evaluated selected result.
 */
static void it_unwinds_every_selected_value_evaluation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        const StaticResultContext *context = setup_selection(selected_caller, "@...\n.out\n.slot\n");
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        StaticResultUse *use = NULL;
        bool success = evaluate_selection(context, result_field("data"), &use);
        if (!failure) { assert(success && use->closed); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && results.status != STATIC_RESULT_OK); }
        failure_at = 0;
        destroy_results();
    }
}

/** A scalar publication retains whole-supplier validation through the original
 * use without reporting a second result bundle or creating received aliases.
 * Every allocation refusal includes publication preparation and teardown.
 */
static void it_publishes_selected_values_with_supplier_proofs_and_unwinds_all_refusals(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        const StaticResultContext *context = setup_selection(selected_caller, "@...\n.out\n.slot\n");
        StaticPublicationPlan plan;
        static_publication_init(&plan, &results, sources.allocator);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        const StaticPublication *publication = NULL;
        bool success = static_publication_resolve(&plan, context, result_field("data"), &publication);
        if (!failure) {
            assert(success && publication && publication->value == integer_type);
            assert(publication->context == NULL && publication->first_output == NULL && publication->output_count == 0);
            assert(publication->use->required_graph->local_complete && publication->use->selection);
            assert(plan.alias_count == 0);
            total = allocation_count - before;
            assert(total > 0);
            const StaticPublication *again;
            assert(static_publication_resolve(&plan, context, result_field("data"), &again) && again == publication);
        } else {
            assert(!success && publication == NULL && plan.status == STATIC_PUBLICATION_OUT_OF_MEMORY);
        }
        failure_at = 0;
        static_publication_destroy(&plan);
        destroy_results();
    }
}

/** Execute scalar identity, phase isolation and counted failure partitions. */
static void run(void) {
    it_reads_one_exact_foreign_result_after_named_input_closure();
    it_keeps_expected_scalar_types_disconnected_from_unresolved_supplier_inputs();
    it_evaluates_nested_selections_under_the_original_lexical_owner();
    it_preserves_selected_partial_constructor_identity_without_runtime_admission();
    it_refuses_nonpublic_or_missing_foreign_selections_without_publishing_sources();
    it_reads_alias_exports_with_checked_foreign_definition_provenance();
    it_rejects_invalid_unselected_outputs_and_closed_consumer_type_conflicts();
    it_unwinds_every_selected_value_evaluation_allocation_refusal();
    it_publishes_selected_values_with_supplier_proofs_and_unwinds_all_refusals();
}
/** Return only after counted teardown of every owner. */
int main(void) { run(); return 0; }
