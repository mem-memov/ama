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

/** Register a pair-defined metadata expression without evaluation. */
static StaticResultUse *prepared_pair(const char *name) {
    const StaticCreationPath *root;
    const StaticResultContext *context;
    StaticResultUse *use;
    assert(static_creation_root(&guards, caller_environment, &root));
    assert(static_result_context(&results, root, &context));
    assert(static_result_use(&results, context, result_pair(name), NULL, &use));
    return use;
}

/** Nested named operands retain their caller scope and prepare only one level. */
static void it_preserves_named_nested_operands_without_evaluation(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/family", "@<\nvalue\n...slot\n@!\n");
    setup_guards("@...\n...built\n.family\n ...slot < .family\n  ...slot < ..integer\n.other\n ...out > ...alias\n", result_target_source);
    initialize_results();
    StaticResultUse *use = prepared_pair("...built");
    assert(static_result_prepare(&results, use));
    const StaticTermApplication *prepared = use->prepared;
    assert(prepared && prepared->input_count == 1 && prepared->first == prepared->last);
    StaticResultUse *child;
    const AstTypeExpression *expression = prepared->first->request->syntax.static_binding->input;
    assert(static_result_use(&results, use->context, expression, NULL, &child));
    assert(prepared->first->value == child->source->value && child->context == use->context);
    assert(child->prepared == NULL && use->application == NULL && results.use_count == 2);
    assert(solver.equation_count == 0 && guards.path_count == 1);
    size_t before = allocation_count;
    assert(static_result_prepare(&results, use) && use->prepared == prepared && allocation_count == before);
    assert(static_result_prepare(&results, child) && child->prepared->input_count == 1);
    assert(results.use_count == 3 && guards.path_count == 1);
    destroy_results();
}

/** A real named producer has requests and a complete formal map, no value application. */
static void it_prepares_named_producers_and_rejects_copied_work_handles(void) {
    setup_results();
    const StaticCreationPath *root;
    const StaticResultContext *context;
    StaticResultUse *use;
    assert(static_creation_root(&guards, caller_environment, &root));
    assert(static_result_context(&results, root, &context));
    assert(static_result_use(&results, context, result_projection(0), NULL, &use));
    assert(static_result_prepare(&results, use));
    assert(use->prepared == NULL && use->producer_request && use->requests->output_count == 1);
    assert(use->producer_request->input_count == 1 && use->application == NULL);
    StaticResultUse copied = *use;
    assert(!static_result_prepare(&results, &copied));
    assert(results.status == STATIC_RESULT_INVALID && solver.equation_count == 0);
    destroy_results();
}

/** Refuse each actual nested operand allocation before publishing preparation. */
static void it_unwinds_every_shallow_operand_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        directory(STATIC_SCOPE_ROOT_DIRECTORY);
        file("generated/static scope/family", "@<\nvalue\n...slot\n@!\n");
        setup_guards("@...\n...built\n.family\n ...slot < ..integer\n.other\n ...out > ...alias\n", result_target_source);
        initialize_results();
        StaticResultUse *use = prepared_pair("...built");
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool success = static_result_prepare(&results, use);
        if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && use->prepared == NULL && results.status == STATIC_RESULT_OUT_OF_MEMORY); }
        failure_at = 0;
        destroy_results();
    }
}

/** Origin evidence can arrive after shallow preparation without guessing a constructor. */
static void it_refines_actual_origins_without_repeating_unchanged_source_work(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/family", "@<\nvalue\n...slot\n@!\n");
    setup_guards("@...\n...built\n...family\n ...slot < ..integer\n...anchor\n.family\n.other\n ...out > ...alias\n", result_target_source);
    initialize_results();
    StaticResultUse *use = prepared_pair("...built");
    StaticApplicationOutcome outcome;
    assert(static_result_apply(&results, use, &outcome));
    assert(outcome == STATIC_APPLY_PENDING && use->application->pending_origin && use->producer == NULL);
    const StaticApplicationRecord *pending = use->application;
    size_t records = applications.count;
    assert(static_result_apply(&results, use, &outcome));
    assert(use->application == pending && applications.count == records && guards.path_count == 1);
    const StaticModule *family = static_module_find(&inventory, project_find(&project, ".family"));
    const StaticTerm *constructor;
    assert(static_term_nominal(&terms, family, NULL, 0, (SourceSpan){0}, &constructor));
    assert(static_constraint_equate(&solver, use->source->head, constructor, use->source->expression->span));
    assert(static_result_apply(&results, use, &outcome));
    assert(use->application != pending && !use->application->pending_origin && use->producer);
    const StaticResultContext *producer = use->producer;
    records = applications.count;
    size_t paths = guards.path_count;
    assert(static_constraint_equate(&solver, use->prepared->first->value, integer_type, use->source->expression->span));
    assert(static_result_apply(&results, use, &outcome) && outcome == STATIC_APPLY_RESOLVED);
    assert(use->producer == producer && applications.count == records && guards.path_count == paths);
    assert(results.definition_use_count == 0 && !use->closed && use->outcome == STATIC_RESULT_PENDING);
    destroy_results();
}

/** Exercise real preparation consumers, malformed identities and every allocation. */
static void run(void) {
    it_shares_only_same_source_environment_operands_across_actual_contexts();
    it_refuses_foreign_operand_sources_receivers_and_cache_count_overflow();
    it_unwinds_every_live_zero_input_preparation_allocation_refusal();
    it_preserves_named_nested_operands_without_evaluation();
    it_prepares_named_producers_and_rejects_copied_work_handles();
    it_unwinds_every_shallow_operand_preparation_allocation_refusal();
    it_refines_actual_origins_without_repeating_unchanged_source_work();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
