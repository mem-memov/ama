/** @file
 * @brief Compare producer-output omission against partial, field, argument and constant namespace contexts.
 */
#include "../common.c"

/** Sections share one independently determined receiver. No member value is
 * executed; the namespace expression itself is the selected static operation.
 */
static const char *without_outputs = "@=\nshared\n.other\n@<\ninput&\n.other\n@>\n...expected\n..integer\n"
    "returned&\n.other\n...partial\n.other\n@!\nanchor&\n..integer\nfield&\n.other\n.other\n constant >> sink\n";
static const char *with_outputs = "@=\nshared\n.other\n ...out > ...expected\n@<\ninput&\n.other\n ...out > ...expected\n"
    "@>\n...expected\n..integer\nreturned&\n.other\n ...out > ...expected\n...partial\n.other\n ...out > ...expected\n"
    "@!\nanchor&\n..integer\nfield&\n.other\n ...out > ...expected\n.other\n ...out > ...expected\n constant >> sink\n";
static const char *constant_target = "@=\nconstant\n...slot\n@<\n...slot\n@>\n...out\n...slot\n@!\n";

/** Original occurrence roles select the expression; no source text is reparsed
 * and no namespace member or runtime constant value is read by publication.
 */
static const AstTypeExpression *context_expression(size_t index) {
    const char *names[] = {"shared", "input", "returned", "field"};
    if (index < 4) { return result_field(names[index]); }
    if (index == 4) {
        const StaticResultDefinition *definition;
        assert(static_result_definition(&results, caller_environment, result_named(caller_module, "...partial"), &definition));
        return definition->source->expression;
    }
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->context == STATIC_CONTEXT_NAMESPACE) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Output syntax uniformly enables producer holes, and an existing receiver
 * closes them. Without output syntax only a static result preserves residuals;
 * ordinary fields require anchors and inputs/namespaces require explicit coverage.
 */
static void it_applies_output_phase_inference_uniformly_across_all_original_contexts(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        for (size_t index = 0; index < 6; index++) {
            setup_publications(mode ? with_outputs : without_outputs, constant_target);
            const StaticResultContext *context = publication_context(caller_environment);
            const StaticPublication *publication = NULL;
            bool success = static_publication_resolve(&publications, context, context_expression(index), &publication);
            assert(success == (mode != 0 || index == 4));
            if (success) {
                assert(publication->value->as.nominal.origin == target_module);
                assert(publication->value->as.nominal.residual_count == (mode ? 0 : 1));
                if (mode) { assert(publication->value->as.nominal.first_capture->value == integer_type); }
            } else {
                assert(publication == NULL && publications.status ==
                    ((index == 1 || index == 5) ? STATIC_PUBLICATION_CONFLICT : STATIC_PUBLICATION_UNRESOLVED));
            }
            assert(caller_module->parameter_count == 0);
            destroy_publications();
        }
    }
}

/** An already applied lexical owner still supplies its captured Type normally;
 * the contextual matrix must not turn that known input into another producer hole.
 */
static void it_preserves_preexisting_owner_captures_when_resolving_result_contexts(void) {
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    const StaticTermEnvironment *environment = result_applied_environment();
    const StaticResultContext *context = publication_context(environment);
    const StaticResultDefinition *definition;
    assert(static_result_definition(&results, environment, result_named(caller_module, "...first"), &definition));
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, definition->source->expression, &publication));
    assert(publication->value == integer_type && publication->output_count == 0);
    destroy_publications();
}

/** Execute the six-context matrix with and without output actions. */
static void run(void) {
    it_applies_output_phase_inference_uniformly_across_all_original_contexts();
    it_preserves_preexisting_owner_captures_when_resolving_result_contexts();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
