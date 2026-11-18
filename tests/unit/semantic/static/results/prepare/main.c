/** @file
 * @brief Verify shallow expression preparation preserves lexical values and all original actions.
 */
#include "../common.c"

/** Register an original root without evaluating its inputs or result fields. */
static StaticResultUse *prepared_root(const char *name) {
    const StaticCreationPath *root;
    const StaticResultContext *context;
    StaticResultUse *use;
    assert(static_creation_root(&guards, caller_environment, &root));
    assert(static_result_context(&results, root, &context));
    assert(static_result_use(&results, context, result_field(name), NULL, &use));
    return use;
}

#include "support/operands.c"

/** Skips count as positions while explicit nested inputs keep the lexical
 * context. No equation, producer context or recursive child preparation occurs.
 */
static void it_preserves_nested_inputs_skips_and_output_provenance_without_evaluation(void) {
    setup_guards("@<\n...type\n@!\nanchor&\n..integer\nfield&\n.other\n ... <\n ... < .other\n  ... < ...type\n ... >\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    StaticResultUse *use = prepared_root("field");
    assert(static_result_prepare(&results, use));
    const StaticTermApplication *prepared = use->prepared;
    assert(prepared && prepared->input_count == 2 && prepared->first->value == NULL);
    assert(prepared->source->output_count == 1 && prepared->first->request->omitted);
    StaticResultUse *child;
    const AstTypeExpression *expression = prepared->last->request->syntax.static_binding->input;
    assert(static_result_use(&results, use->context, expression, NULL, &child));
    assert(prepared->last->value == child->source->value && child->context == use->context);
    assert(child->prepared == NULL && use->application == NULL && results.use_count == 2);
    assert(solver.equation_count == 0 && guards.path_count == 1 && results.context_count == 1);
    size_t before = allocation_count;
    assert(static_result_prepare(&results, use) && use->prepared == prepared && allocation_count == before);
    assert(static_result_prepare(&results, child));
    assert(child->prepared->input_count == 1 && results.use_count == 3 && guards.path_count == 1);
    destroy_results();
}

/** Zero-input producers retain outputs without manufacturing an operand array. */
static void it_prepares_zero_inputs_and_rejects_copied_work_handles(void) {
    setup_results();
    StaticResultUse *use = prepared_root("first");
    assert(static_result_prepare(&results, use));
    assert(use->prepared->first == NULL && use->prepared->source->output_count == 1);
    const StaticTermEnvironment *applied = result_applied_environment();
    const StaticCreationPath *root;
    const StaticResultContext *context;
    StaticResultDefinitionUse *definition;
    StaticResultUse *rhs;
    assert(static_creation_root(&guards, applied, &root));
    assert(static_result_context(&results, root, &context));
    assert(static_result_definition_use(&results, context, result_named(caller_module, "...first"),
        NULL, (SourceSpan){0}, &definition));
    assert(static_result_use(&results, context, definition->definition->source->expression,
        definition->path, &rhs));
    assert(static_result_prepare(&results, rhs));
    assert(rhs->prepared->environment == applied && rhs->prepared->head == integer_type);
    StaticResultUse copied = *use;
    assert(!static_result_prepare(&results, &copied));
    assert(results.status == STATIC_RESULT_INVALID && solver.equation_count == 0);
    destroy_results();
}

/** Every observed preparation allocation may fail with no published prepared
 * operator; partial child/source records remain owned and destructible.
 */
static void it_unwinds_every_shallow_operand_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_guards("@!\nanchor&\n..integer\nfield&\n.other\n ... < ..integer\n", result_target_source);
        static_result_init(&results, &guards, sources.allocator);
        StaticResultUse *use = prepared_root("field");
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool success = static_result_prepare(&results, use);
        if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && use->prepared == NULL && results.status == STATIC_RESULT_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_results();
    }
}

/** Independent origin evidence replaces one raw snapshot; unchanged retries
 * preserve it. A known symbolic producer is guarded before outputs are read.
 */
static void it_refines_actual_origins_without_repeating_unchanged_source_work(void) {
    setup_guards("@<\n...family\n ...slot\n@!\nanchor&\n..integer\nfield&\n...family\n ... < ..integer\n ... >\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    StaticResultUse *use = prepared_root("field");
    StaticApplicationOutcome outcome;
    bool applied = static_result_apply(&results, use, &outcome);
    if (!applied) { fprintf(stderr, "Application failure: %s\n", results.error.message); }
    assert(applied);
    assert(outcome == STATIC_APPLY_PENDING && use->application->pending_origin && use->producer == NULL);
    const StaticApplicationRecord *pending = use->application;
    size_t records = applications.count;
    assert(static_result_apply(&results, use, &outcome));
    assert(use->application == pending && applications.count == records && guards.path_count == 1);
    assert(static_constraint_equate(&solver, use->source->head, target_constructor, use->source->expression->span));
    assert(static_result_apply(&results, use, &outcome));
    assert(use->application != pending && !use->application->pending_origin && use->producer != NULL);
    const StaticResultContext *producer = use->producer;
    records = applications.count;
    size_t paths = guards.path_count;
    assert(static_constraint_equate(&solver, use->prepared->first->value, integer_type, use->source->expression->span));
    assert(static_result_apply(&results, use, &outcome));
    assert(outcome == STATIC_APPLY_RESOLVED && use->producer == producer);
    assert(applications.count == records && guards.path_count == paths);
    assert(results.definition_use_count == 0 && !use->closed && use->outcome == STATIC_RESULT_PENDING);
    destroy_results();
}

/** Only a whole expression Type, not its selected alias, supplies the nominal
 * origin for exact constructor recovery against the declared residual names.
 */
static void it_recovers_a_constructor_from_an_independent_whole_expression_type(void) {
    setup_guards("@<\n...family\n ...slot\n@!\nanchor&\n..integer\nfield&\n...family\n ... < ..integer\n", result_target_source);
    static_result_init(&results, &guards, sources.allocator);
    StaticResultUse *use = prepared_root("field");
    StaticTermCapture capture = {.formal = application_formal(target_module, 0), .value = integer_type};
    const StaticTerm *actual;
    assert(static_term_nominal(&terms, target_module, &capture, 1, use->source->expression->span, &actual));
    assert(static_constraint_equate(&solver, use->source->value, actual, use->source->expression->span));
    StaticApplicationOutcome outcome;
    assert(static_result_apply(&results, use, &outcome));
    const StaticTerm *value;
    StaticConstraintOutcome solved;
    assert(static_constraint_value(&solver, use->source->head, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == target_constructor);
    assert(static_constraint_value(&solver, use->application->value, &value, &solved));
    assert(solved == STATIC_CONSTRAINT_COMPLETE && value == actual && use->producer != NULL);
    destroy_results();
}

/** Execute source-position, shallow-depth, foreign-handle and failure evidence. */
static void run(void) {
    it_shares_only_same_source_environment_operands_across_actual_contexts();
    it_refuses_foreign_operand_sources_receivers_and_cache_count_overflow();
    it_unwinds_every_live_zero_input_preparation_allocation_refusal();
    it_preserves_nested_inputs_skips_and_output_provenance_without_evaluation();
    it_prepares_zero_inputs_and_rejects_copied_work_handles();
    it_unwinds_every_shallow_operand_preparation_allocation_refusal();
    it_refines_actual_origins_without_repeating_unchanged_source_work();
    it_recovers_a_constructor_from_an_independent_whole_expression_type();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
