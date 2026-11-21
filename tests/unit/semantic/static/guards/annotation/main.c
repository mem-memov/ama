/** @file
 * @brief Verify exact source-only annotation frames and separation from actual compilation roots.
 */
#include "../../results/common.c"

/** One free owner Type and one selected producer input remain distinct scopes. */
static const char *caller = "@<\nargument\n.type\n@!\nvalue&\n/test/fixture/v1.0.0/src/other\n .slot < .type\n .out\nanchor&\n//integer\n";
static const char *target = "@...\n.out\n.slot\n";

/** Source facts may identify an owner without requesting its actual body. */
static void setup_annotation(void) {
    setup_guards(caller, target);
    initialize_results();
}

/** Exact original syntax supplies one immutable lexical predecessor; a later
 * selected compilation root has its own identity and admission boundary.
 */
static void it_preserves_original_annotation_identity_without_creating_an_actual_scope(void) {
    setup_annotation();
    const AstTypeExpression *expression = result_field("value");
    const StaticCreationPath *annotation, *again, *root;
    assert(static_creation_annotation(&guards, caller_environment, expression, &annotation));
    assert(annotation->source_only && !annotation->parent && !annotation->required_from);
    assert(annotation->owner->environment == caller_environment && annotation->site.start == expression->span.start);
    assert(results.context_count == 0 && result_registry.use_count == 0 && guards.path_count == 1);
    StaticRegistryCheckState checks;
    assert(static_registry_refresh_checks(&result_registry, &checks));
    assert(result_registry.use_count == 0 && result_registry.candidate_count == 0);
    size_t before = allocation_count;
    assert(static_creation_annotation(&guards, caller_environment, expression, &again));
    assert(again == annotation && allocation_count == before);
    assert(static_creation_root(&guards, caller_environment, &root));
    assert(root != annotation && !root->source_only && root->owner == annotation->owner);
    assert(results.context_count == 0 && result_registry.use_count == 0 && guards.path_count == 2);
    destroy_results();
}

/** A real producer requirement retains the lexical edge. Registering its
 * context must not promote the annotation predecessor into an ordinary scope.
 * Neither result nor registry entry APIs accept the annotation itself.
 */
static void it_checks_suppliers_without_promoting_annotation_frames_to_actual_contexts(void) {
    for (size_t direct_registry = 0; direct_registry < 2; direct_registry++) {
        setup_annotation();
        const AstTypeExpression *expression = result_field("value");
        const StaticCreationPath *annotation, *producer;
        assert(static_creation_annotation(&guards, caller_environment, expression, &annotation));
        StaticTermCapture input = {.formal = application_formal(target_module, 0), .value = integer_type};
        StaticCreationOutcome outcome;
        assert(static_creation_producer_enter(&guards, annotation, expression, &input, 1, &producer, &outcome));
        assert(outcome == STATIC_CREATION_ENTERED && producer->parent == annotation && !producer->source_only);
        const StaticResultContext *context;
        assert(static_result_context(&results, producer, &context));
        assert(results.context_count == 1 && result_registry.use_count == 1 && result_registry.candidate_count == 1);
        assert(context->registry_use->candidate->key->definition == target_module);
        size_t before = allocation_count;
        if (direct_registry) {
            StaticRegistryUse *use = context->registry_use;
            StaticRegistryKeyState state = STATIC_REGISTRY_KEY_CLOSED;
            assert(!static_registry_use(&result_registry, annotation, &use, &state));
            assert(use == context->registry_use && state == STATIC_REGISTRY_KEY_CLOSED);
        } else {
            const StaticResultContext *refused = context;
            assert(!static_result_context(&results, annotation, &refused) && refused == context);
        }
        assert(allocation_count == before && results.context_count == 1 && result_registry.use_count == 1);
        destroy_results();
    }
}

/** Copies, foreign scopes and nested operands are not original annotation
 * roots. Every refusal precedes allocation and preserves its output receiver.
 */
static void it_refuses_nonoriginal_annotation_requests_without_allocating(void) {
    for (size_t mode = 0; mode < 5; mode++) {
        setup_annotation();
        const AstTypeExpression *expression = result_field("value");
        AstTypeExpression copied = *expression;
        StaticTermEnvironment environment = *caller_environment;
        const StaticCreationPath *output = NULL;
        const StaticTermEnvironment *foreign;
        assert(static_term_environment(&terms, target_module, NULL, &foreign));
        size_t before = allocation_count;
        assert(!static_creation_annotation(&guards, mode == 0 ? &environment : mode == 1 ? foreign : caller_environment,
            mode == 2 ? &copied : mode == 3 ? expression->first->input : expression,
            mode == 4 ? NULL : &output));
        assert(output == NULL && guards.status == STATIC_GUARD_INVALID && allocation_count == before && guards.path_count == 0);
        destroy_results();
    }
}

/** Refuse each measured allocation while preserving a destructible provenance
 * owner and an unchanged public output, including failures after frame retention.
 */
static void it_unwinds_every_annotation_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_annotation();
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        const StaticCreationPath *output = NULL;
        bool success = static_creation_annotation(&guards, caller_environment, result_field("value"), &output);
        if (!failure) { assert(success && output); total = allocation_count - before; assert(total > 0); }
        else { assert(!success && output == NULL && guards.status == STATIC_GUARD_OUT_OF_MEMORY); }
        failure_at = 0;
        assert(results.context_count == 0 && result_registry.use_count == 0);
        destroy_results();
    }
}

/** Execute provenance, scope admission, exact membership and refusal partitions. */
static void run(void) {
    it_preserves_original_annotation_identity_without_creating_an_actual_scope();
    it_checks_suppliers_without_promoting_annotation_frames_to_actual_contexts();
    it_refuses_nonoriginal_annotation_requests_without_allocating();
    it_unwinds_every_annotation_preparation_allocation_refusal();
}
/** Return only after counted teardown of every borrowed/owned component. */
int main(void) { run(); return 0; }
