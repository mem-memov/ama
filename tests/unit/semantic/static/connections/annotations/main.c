/** @file
 * @brief Check exact field annotation facts, actual validation and operation refusal ownership.
 */
#include "../typed/common.c"

/** Metadata scope field facts preserve actual context/body separation. */
static void it_applies_original_annotation_facts_without_creating_an_ordinary_context(void) {
    setup_typed();
    const StaticTypedScope *scope;
    bool success = static_connection_scope(&connections, typed_environment(integer_type), &scope);
    assert(success);
    bool changed;
    success = static_connection_annotations(&connections, scope, &changed);
    assert(success && changed && results.context_count == 0 && guards.path_count == 0);
    const StaticTypedField *known = typed_field(scope, "known"), *values = typed_field(scope, "values");
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    success = static_constraint_value(&solver, known->type, &value, &outcome);
    assert(success && outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    success = static_constraint_value(&solver, values->type, &value, &outcome);
    assert(success && outcome == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    assert(typed_field(scope, "fresh")->type_source == NULL);
    assert(typed_field(scope, "worker")->type_source->application->value->category == STATIC_CATEGORY_TYPE);
    success = static_connection_annotations(&connections, scope, &changed);
    assert(success && !changed && connections.context_count == 0 && publications.count == 0);
    destroy_connections();
}

/** Actual field validation closes Type requirements but leaves ordinary body
 * and capability checks unadmitted in the canonical registry.
 */
static void it_validates_known_fields_only_under_their_actual_incoming_context(void) {
    setup_connections("@<\nargument\n//integer\n@>\nresult\n//text\n@!\nfield\n//integer\n", "@!\n");
    const StaticResultContext *actual = publication_context(caller_environment);
    StaticResultOutcome outcome;
    bool success = static_connection_validate_fields(&connections, actual, &outcome);
    assert(success && outcome == STATIC_RESULT_READY && connections.context_count == 1);
    assert(connections.first_context->scope->field_count == 3);
    assert(actual->registry_use->candidate && !actual->registry_use->candidate->local_complete);
    success = static_connection_validate_fields(&connections, actual, &outcome);
    assert(success && outcome == STATIC_RESULT_READY && connections.context_count == 1);
    destroy_connections();
}

/** Required inference holes stay Pending even after unrelated declarations have
 * known Types. No literal/default runtime value supplies a guessed field Type.
 */
static void it_retains_inferred_and_generic_field_requirements_without_inventing_anchors(void) {
    setup_typed();
    const StaticResultContext *actual = typed_actual();
    StaticResultOutcome outcome;
    bool success = static_connection_validate_fields(&connections, actual, &outcome);
    assert(success && outcome == STATIC_RESULT_PENDING && connections.context_count == 1);
    const StaticTypedScope *scope = connections.first_context->scope;
    const StaticTerm *value = NULL;
    StaticConstraintOutcome closed;
    success = static_constraint_value(&solver, typed_field(scope, "fresh")->type, &value, &closed);
    assert(success && closed == STATIC_CONSTRAINT_PENDING && value == NULL);
    assert(!actual->registry_use->candidate || !actual->registry_use->candidate->local_complete);
    destroy_connections();
}

/** An independent static supplier can provide facts for a nominal metadata
 * scope; a later actual caller must retain its own supplier validation edge.
 */
static void it_retains_real_export_validation_dependencies_after_metadata_type_resolution(void) {
    setup_connections("@!\nexported\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n//integer\n");
    const StaticTermEnvironment *environment;
    bool success = static_term_environment(&terms, caller_module, caller_constructor, &environment);
    assert(success);
    const StaticTypedScope *scope;
    success = static_connection_scope(&connections, environment, &scope);
    assert(success);
    bool changed;
    success = static_connection_annotations(&connections, scope, &changed);
    assert(success && changed && connections.context_count == 0);
    for (const StaticResultContext *context = results.first_context; context; context = context->next) {
        assert(context->creation->owner->environment->definition != caller_module);
    }
    const StaticTerm *value;
    StaticConstraintOutcome closed;
    success = static_constraint_value(&solver, typed_field(scope, "exported")->type, &value, &closed);
    assert(success && closed == STATIC_CONSTRAINT_COMPLETE && value == integer_type);
    const StaticResultContext *actual = publication_context(environment);
    size_t dependencies = result_registry.dependency_count;
    StaticResultOutcome outcome;
    success = static_connection_validate_fields(&connections, actual, &outcome);
    assert(success && outcome == STATIC_RESULT_READY && result_registry.dependency_count > dependencies);
    assert(connections.context_count == 1 && connections.first_context->scope == scope);
    destroy_connections();
}

/** Copied scopes/contexts and missing receivers cannot certify field work. */
static void it_refuses_foreign_annotation_scopes_contexts_and_missing_outputs(void) {
    for (size_t kind = 0; kind < 4; kind++) {
        setup_typed();
        const StaticResultContext *actual = typed_actual();
        const StaticTypedScope *scope;
        bool success = static_connection_scope(&connections, caller_environment, &scope);
        assert(success);
        StaticTypedScope copied = *scope;
        StaticResultContext foreign = *actual;
        bool changed = true;
        StaticResultOutcome outcome = STATIC_RESULT_READY;
        if (kind < 2) { success = static_connection_annotations(&connections, kind ? scope : &copied, kind ? NULL : &changed); }
        else { success = static_connection_validate_fields(&connections, kind == 2 ? &foreign : actual, kind == 3 ? NULL : &outcome); }
        assert(!success && changed && outcome == STATIC_RESULT_READY && connections.status == STATIC_CONNECTION_INVALID);
        destroy_connections();
    }
}

/** Every measured annotation/evaluation allocation may refuse without exposing
 * an outcome; all partial publication/result/connection records remain owned.
 */
static void it_unwinds_every_annotation_fact_and_actual_validation_allocation_refusal(void) {
    for (size_t phase = 0; phase < 2; phase++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_typed();
            const StaticResultContext *actual = phase ? typed_actual() : NULL;
            const StaticTypedScope *scope;
            bool success = phase ? static_connection_context(&connections, actual, &scope) :
                static_connection_scope(&connections, typed_environment(integer_type), &scope);
            assert(success);
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool changed = true;
            StaticResultOutcome outcome = STATIC_RESULT_READY;
            success = phase ? static_connection_validate_fields(&connections, actual, &outcome) :
                static_connection_annotations(&connections, scope, &changed);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && connections.status == STATIC_CONNECTION_OUT_OF_MEMORY);
                assert(changed && outcome == STATIC_RESULT_READY);
            }
            failure_at = 0;
            destroy_connections();
        }
    }
}

/** Execute independent metadata, actual paths, required closure and refusal cases. */
static void run(void) {
    it_applies_original_annotation_facts_without_creating_an_ordinary_context();
    it_validates_known_fields_only_under_their_actual_incoming_context();
    it_retains_inferred_and_generic_field_requirements_without_inventing_anchors();
    it_retains_real_export_validation_dependencies_after_metadata_type_resolution();
    it_refuses_foreign_annotation_scopes_contexts_and_missing_outputs();
    it_unwinds_every_annotation_fact_and_actual_validation_allocation_refusal();
}
/** Counted teardown completes before success. */
int main(void) { run(); return 0; }
