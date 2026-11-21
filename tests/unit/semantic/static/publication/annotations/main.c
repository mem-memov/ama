/** @file
 * @brief Verify source-only ordinary annotation equations and independent export evidence.
 */
#include "../common.c"
#include "semantic/static/publication/types.h"
#include "semantic/static/publication/ordinary/projections/internal.h"

/** A generic ordinary path needs per-field Type holes, never a residual value. */
static void setup_annotations(void) {
    setup_publications("@!\nfield\n/test/fixture/v1.0.0/src/other\nanchor&\n//integer\n", "@<\nvalue\n.slot\n@!\n");
}

/** Bind one original annotation without registering any actual incoming scope. */
static StaticOrdinaryType *annotation(void) {
    StaticOrdinaryType *type;
    bool success = static_ordinary_source(&publications, caller_environment, result_field("field"), &type);
    assert(success && type->context == NULL && type->environment == caller_environment);
    return type;
}

/** Metadata facts can infer field arguments; actual validation remains a
 * separate context/use with the same original environment/value identity.
 */
static void it_infers_field_annotation_arguments_without_creating_or_validating_an_ordinary_body(void) {
    setup_annotations();
    StaticOrdinaryType *type = annotation();
    assert(type->source == NULL && type->annotation && results.context_count == 0 && results.use_count == 0);
    bool changed = false;
    bool success = static_ordinary_equations(&publications, type, &changed);
    assert(success && changed && type->application && type->prepared);
    assert(results.context_count == 0 && results.use_count == 0 && guards.path_count == 0);
    assert(publications.count == 0 && type->publication == NULL);
    const StaticTerm *instance = type->application->value;
    assert(instance->category == STATIC_CATEGORY_TYPE && instance->as.nominal.origin == target_module);
    const StaticTerm *hole = instance->as.nominal.first_capture->value;
    assert(hole->kind == STATIC_TERM_VARIABLE && hole->as.variable.role == STATIC_VARIABLE_FIELD);
    success = static_constraint_equate(&solver, hole, integer_type, hole->site);
    assert(success);
    success = static_ordinary_equations(&publications, type, &changed);
    assert(success && !changed);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    success = static_constraint_value(&solver, type->value, &value, &outcome);
    assert(success && outcome == STATIC_CONSTRAINT_COMPLETE && value->as.nominal.first_capture->value == integer_type);
    assert(results.context_count == 0 && guards.path_count == 0);
    const StaticResultContext *actual = publication_context(caller_environment);
    StaticOrdinaryType *use;
    success = static_ordinary_type(&publications, actual, type->expression, &use);
    assert(success && use != type && use->value == type->value && use->annotation == type->annotation);
    const StaticTerm *checked;
    success = static_ordinary_resolve(&publications, use, &checked);
    assert(success && checked == value && use->publication && type->publication == NULL);
    assert(use->source->producer && results.context_count == 2);
    destroy_publications();
}

/** Supplied nominal captures substitute exact original Type parameters without
 * creating actual contexts; even a known Type is not an actual-use certificate.
 */
static void it_substitutes_nominal_annotation_scopes_without_inventing_validation_provenance(void) {
    setup_publications("@<\nargument\n.type\n@!\nfield\n.type\nanchor&\n//integer\n", "@!\n");
    StaticTermCapture capture = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    bool success = static_term_nominal(&terms, caller_module, &capture, 1, (SourceSpan){0}, &owner);
    assert(success);
    success = static_term_environment(&terms, caller_module, owner, &environment);
    assert(success);
    StaticOrdinaryType *type, *again;
    success = static_ordinary_source(&publications, environment, result_field("field"), &type);
    assert(success && type->annotation->head == integer_type && type->source == NULL);
    bool changed;
    success = static_ordinary_equations(&publications, type, &changed);
    assert(success && changed && results.context_count == 0 && guards.path_count == 0);
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    success = static_constraint_value(&solver, type->value, &value, &outcome);
    assert(success && outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    size_t before = allocation_count;
    failure_at = before + 1;
    success = static_ordinary_source(&publications, environment, type->expression, &again);
    failure_at = 0;
    assert(success && again == type && allocation_count == before && publications.type_count == 1);
    value = NULL;
    success = static_ordinary_resolve(&publications, type, &value);
    assert(!success && value == NULL && publications.status == STATIC_PUBLICATION_INVALID);
    assert(results.status == STATIC_RESULT_OK && solver.status == STATIC_CONSTRAINT_OK);
    destroy_publications();
}

/** Source-only export facts retain a lexical graph predecessor, but cannot
 * fabricate the using ordinary module's context or certify its runtime body.
 */
static void it_resolves_independent_exports_without_manufacturing_an_ordinary_caller_context(void) {
    setup_publications("@!\nfield\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n//integer\n");
    StaticOrdinaryType *type = annotation();
    bool changed;
    bool success = static_ordinary_equations(&publications, type, &changed);
    assert(success && changed && type->projection && type->projection->context && !type->publication);
    assert(type->annotation_path->source_only && type->projection->context->creation->owner->value == NULL);
    for (const StaticResultContext *context = results.first_context; context; context = context->next) {
        assert(context->creation->owner->environment->definition != caller_module);
    }
    const StaticTerm *value;
    success = static_ordinary_resolve(&publications, type, &value);
    assert(success && value == integer_type);
    const StaticResultContext *actual = publication_context(caller_environment);
    StaticOrdinaryType *use;
    success = static_ordinary_type(&publications, actual, type->expression, &use);
    assert(success && use != type && use->value == type->value);
    size_t dependencies = result_registry.dependency_count;
    success = static_ordinary_equations(&publications, use, &changed);
    assert(success && changed && result_registry.dependency_count > dependencies);
    assert(use->projection->context != type->projection->context && !use->publication);
    assert(use->projection->context->registry_use->candidate == type->projection->context->registry_use->candidate);
    assert(use->projection->context->creation->parent == actual->creation);
    success = static_ordinary_resolve(&publications, use, &value);
    assert(success && value == integer_type && use->publication != type->publication);
    assert(use->publication->context == NULL && use->publication->value == integer_type);
    success = static_ordinary_equations(&publications, use, &changed);
    assert(success && !changed);
    destroy_publications();
}

/** Known interface facts do not authorize a repeated ordinary definition while
 * its incoming key is unknown. Equal later keys resume; different keys refuse,
 * even though metadata already knows the selected field's complete Type.
 */
static void it_preserves_pending_and_unequal_ancestry_after_source_only_annotation_inference(void) {
    for (size_t different = 0; different < 2; different++) {
        setup_publications("@<\nargument\n.slot\n@!\nnext&\n/test/fixture/v1.0.0/src/main\nanchor&\n//integer\ntext anchor&\n//text\n", "@!\n");
        StaticOrdinaryType *metadata;
        bool success = static_ordinary_source(&publications, caller_environment, result_field("next"), &metadata);
        assert(success);
        bool changed;
        success = static_ordinary_equations(&publications, metadata, &changed);
        assert(success && changed && guards.path_count == 0 && results.context_count == 0);
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
        const StaticTerm *expected;
        success = static_term_nominal(&terms, caller_module, &input, 1, (SourceSpan){0}, &expected);
        assert(success);
        success = static_constraint_equate(&solver, metadata->value, expected, metadata->expression->span);
        assert(success);
        success = static_ordinary_equations(&publications, metadata, &changed);
        assert(success && guards.path_count == 0 && results.context_count == 0);
        const StaticResultContext *actual = publication_context(caller_environment);
        StaticOrdinaryType *use;
        success = static_ordinary_type(&publications, actual, metadata->expression, &use);
        assert(success);
        StaticResultOutcome outcome;
        success = static_ordinary_evaluate(&publications, use, &outcome);
        assert(success && outcome == STATIC_RESULT_PENDING && use->source->producer == NULL);
        assert(guards.path_count == 1 && results.context_count == 1);
        const StaticTerm *argument = integer_type;
        if (different) {
            const StaticModule *text = static_module_find(&inventory, project_find(&project, "//text"));
            success = static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &argument);
            assert(success);
        }
        success = static_constraint_equate(&solver, application_symbol(".slot"), argument, metadata->expression->span);
        assert(success);
        const StaticTerm *value = NULL;
        success = static_ordinary_resolve(&publications, use, &value);
        assert(success == (different == 0));
        if (success) { assert(value == expected); }
        else {
            assert(value == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
            assert(guards.repeated_ancestor == actual->creation && guards.rejected_creation_key);
        }
        destroy_publications();
    }
}

/** A known annotation consumer is not an input anchor for its static supplier. */
static void it_refuses_unresolved_export_inputs_without_using_consumer_type_evidence(void) {
    setup_publications("@!\nfield\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n.slot\n.anchor\n//integer\n");
    StaticOrdinaryType *type = annotation();
    bool success = static_constraint_equate(&solver, type->value, integer_type, type->expression->span);
    assert(success);
    bool changed = true;
    success = static_ordinary_equations(&publications, type, &changed);
    assert(success && changed && publications.status == STATIC_PUBLICATION_OK);
    assert(type->publication == NULL && publications.count == 0);
    const StaticTerm *hole = type->projection->request->inputs[0].value, *value;
    StaticConstraintOutcome known;
    assert(static_constraint_value(&solver, hole, &value, &known) && known == STATIC_CONSTRAINT_PENDING);
    success = static_ordinary_equations(&publications, type, &changed);
    assert(success && !changed);
    value = NULL;
    assert(!static_ordinary_resolve(&publications, type, &value));
    assert(value == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
    destroy_publications();
}

/** Source metadata does not satisfy actual-use evaluation, and copied syntax,
 * environments or records cannot impersonate registered original identities.
 */
static void it_refuses_foreign_metadata_handles_and_missing_actual_validation_contexts(void) {
    for (size_t kind = 0; kind < 7; kind++) {
        setup_annotations();
        StaticOrdinaryType *type = annotation();
        StaticOrdinaryType copied = *type;
        StaticTermEnvironment environment = *caller_environment;
        AstTypeExpression expression = *type->expression;
        const StaticTerm *value = NULL;
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        bool changed = true, success;
        if (kind == 0) { success = static_ordinary_source(&publications, &environment, type->expression, &type); }
        else if (kind == 1) { success = static_ordinary_source(&publications, caller_environment, &expression, &type); }
        else if (kind == 2) { success = static_ordinary_equations(&publications, &copied, &changed); }
        else if (kind == 3) { success = static_ordinary_equations(&publications, type, NULL); }
        else if (kind == 4) { success = static_ordinary_evaluate(&publications, type, &outcome); }
        else if (kind == 5) { success = static_ordinary_resolve(&publications, type, &value); }
        else { success = static_ordinary_source(&publications, caller_environment, type->expression, NULL); }
        assert(!success && changed && value == NULL && outcome == STATIC_RESULT_PENDING);
        assert(publications.status == STATIC_PUBLICATION_INVALID);
        destroy_publications();
    }
}

/** Every measured source preparation and normal/export equation allocation may
 * refuse; failed operations retain caller outputs and singly owned partial work.
 */
static void it_unwinds_every_source_annotation_and_equation_allocation_refusal(void) {
    for (size_t phase = 0; phase < 3; phase++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            if (phase == 2) {
                setup_publications("@!\nfield\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n//integer\n");
            } else { setup_annotations(); }
            StaticOrdinaryType *type = phase ? annotation() : NULL;
            bool changed = true;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = phase ? static_ordinary_equations(&publications, type, &changed) :
                static_ordinary_source(&publications, caller_environment, result_field("field"), &type);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && publications.status == STATIC_PUBLICATION_OUT_OF_MEMORY);
                assert(phase ? changed : type == NULL);
            }
            failure_at = 0;
            destroy_publications();
        }
    }
}

/** Run finite facts, actual provenance, independent closure and all refusal paths. */
static void run(void) {
    it_infers_field_annotation_arguments_without_creating_or_validating_an_ordinary_body();
    it_substitutes_nominal_annotation_scopes_without_inventing_validation_provenance();
    it_resolves_independent_exports_without_manufacturing_an_ordinary_caller_context();
    it_preserves_pending_and_unequal_ancestry_after_source_only_annotation_inference();
    it_refuses_unresolved_export_inputs_without_using_consumer_type_evidence();
    it_refuses_foreign_metadata_handles_and_missing_actual_validation_contexts();
    it_unwinds_every_source_annotation_and_equation_allocation_refusal();
}
/** Counted teardown must finish before success. */
int main(void) { run(); return 0; }
