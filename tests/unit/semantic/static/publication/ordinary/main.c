/** @file
 * @brief Verify exact named exports without ordinary consumer-driven factory inference.
 */
#include "../common.c"
#include "semantic/static/publication/ordinary/internal.h"

/** Prepare a real ordinary field through its original checked incoming context. */
static StaticOrdinaryType *ordinary(const char *name) {
    const StaticResultContext *context = publication_context(caller_environment);
    StaticOrdinaryType *type;
    bool success = static_ordinary_type(&publications, context, result_field(name), &type);
    assert(success);
    return type;
}

/** A selector names a foreign export without introducing any caller placeholder. */
static void it_resolves_named_exports_as_concrete_types_without_producer_instances(void) {
    setup_publications("@!\nfirst\n/test/fixture/v1.0.0/src/other\n .out\nsecond\n/test/fixture/v1.0.0/src/other\n .out\n",
        "@...\n.out\n//integer\n");
    StaticOrdinaryType *first = ordinary("first"), *second = ordinary("second");
    assert(first != second && first->value != second->value && first->source && second->source);
    assert(caller_module->parameter_count == 0 && caller_module->first == NULL);
    const StaticTerm *first_type, *second_type;
    bool success = static_ordinary_resolve(&publications, first, &first_type);
    assert(success);
    success = static_ordinary_resolve(&publications, second, &second_type);
    assert(success && first_type == integer_type && second_type == first_type);
    assert(first->publication != second->publication && first->publication->value == integer_type);
    assert(first->publication->context == NULL && second->publication->context == NULL);
    assert(first->source->producer->creation->owner->value == NULL);
    assert(first->source->producer->registry_use->candidate == second->source->producer->registry_use->candidate);
    assert(result_registry.dependency_count > 0);
    StaticOrdinaryType *again = ordinary("first");
    assert(again == first && publications.type_count == 2);
    destroy_publications();
}

/** A desired consumer Type is not evidence about a factory's unknown parameter. */
static void it_refuses_open_suppliers_even_when_the_consumer_type_is_already_known(void) {
    setup_publications("@!\nvalue\n/test/fixture/v1.0.0/src/other\n .out\n",
        "@...\n.out\n.slot\n.anchor\n//integer\n");
    StaticOrdinaryType *type = ordinary("value");
    bool success = static_constraint_equate(&solver, type->value, integer_type, type->expression->span);
    assert(success);
    const StaticTerm *resolved = NULL;
    success = static_ordinary_resolve(&publications, type, &resolved);
    assert(!success && resolved == NULL && publications.status == STATIC_PUBLICATION_UNRESOLVED);
    assert(type->publication == NULL && publications.count == 0);
    assert(type->source->producer->registry_use->key_state == STATIC_REGISTRY_KEY_PENDING);
    destroy_publications();
}

/** Other Template outputs do not invalidate a selected Type; selecting the
 * residual constructor itself is a storage/category error.
 */
static void it_distinguishes_selected_type_outputs_from_other_template_outputs(void) {
    for (size_t select_family = 0; select_family < 2; select_family++) {
        fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
        file("generated/static scope/packages/test/fixture/v1.0.0/src/family", "@<\nvalue\n.slot\n@!\n");
        setup_publications(select_family ? "@!\nvalue\n/test/fixture/v1.0.0/src/other\n .family\n" : "@!\nvalue\n/test/fixture/v1.0.0/src/other\n .out\n",
            "@...\n.out\n//integer\n.family\n/test/fixture/v1.0.0/src/family\n");
        StaticOrdinaryType *type = ordinary("value");
        const StaticTerm *resolved = NULL;
        bool success = static_ordinary_resolve(&publications, type, &resolved);
        assert(success == (select_family == 0));
        if (success) {
            assert(resolved == integer_type && type->publication->output_count == 0);
            assert(type->source->required_graph->work_count == 2 && type->source->required_graph->local_complete);
        }
        else { assert(resolved == NULL && publications.status == STATIC_PUBLICATION_CONFLICT); }
        destroy_publications();
    }
}

/** Foreign parameter names are not public exports, even when their spelling exists. */
static void it_rejects_unknown_and_nonpublic_export_names_without_adding_caller_symbols(void) {
    const char *callers[] = {"@!\nvalue\n/test/fixture/v1.0.0/src/other\n .missing\n", "@!\nvalue\n/test/fixture/v1.0.0/src/other\n .slot\n"};
    for (size_t kind = 0; kind < 2; kind++) {
        setup_publications(callers[kind], "@...\n.out\n.slot\n.anchor\n//integer\n");
        const StaticResultContext *context = publication_context(caller_environment);
        StaticOrdinaryType *type = NULL;
        bool success = static_ordinary_type(&publications, context, result_field("value"), &type);
        assert(!success && type == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
        assert(caller_module->first == NULL && publications.type_count == 0);
        destroy_publications();
    }
}

/** Normal fields keep contextual parameter inference without becoming static exports. */
static void it_keeps_normal_ordinary_type_inference_and_actual_source_provenance(void) {
    setup_publications("@<\nargument\n.type\n@!\nanchor&\n//integer\nvalue\n.type\n", "@!\n");
    StaticOrdinaryType *type = ordinary("value");
    StaticResultOutcome outcome;
    bool success = static_ordinary_evaluate(&publications, type, &outcome);
    assert(success && type->producer == NULL && type->source && !type->source->closed);
    const StaticTerm *unknown;
    StaticConstraintOutcome unknown_state;
    success = static_constraint_value(&solver, type->value, &unknown, &unknown_state);
    assert(success && unknown_state == STATIC_CONSTRAINT_PENDING);
    success = static_constraint_equate(&solver, application_symbol(".type"), integer_type, type->expression->span);
    assert(success);
    const StaticTerm *resolved;
    success = static_ordinary_resolve(&publications, type, &resolved);
    assert(success && resolved == integer_type && type->publication->value == integer_type);
    assert(type->publication->context == NULL);
    destroy_publications();
}

/** Namespace admission independently rejects a marker supplied through a Type
 * equation, even before a caller asks the runtime field binder to validate it.
 */
static void it_refuses_substituted_marker_namespaces_without_publishing_a_value(void) {
    setup_publications("@<\nargument\n.type\n@!\nanchor&\n//integer\nvalue\n.type\n", "@...\n");
    assert(target_module->definition->syntax.sections.kind == MODULE_MARKER);
    StaticOrdinaryType *type = ordinary("value");
    assert(static_constraint_equate(&solver, application_symbol(".type"),
        target_constructor, type->expression->span));
    const StaticTerm *resolved = integer_type;
    assert(!static_ordinary_resolve(&publications, type, &resolved));
    assert(resolved == integer_type && publications.status == STATIC_PUBLICATION_CONFLICT);
    assert(strstr(publications.error.message, "marker") != NULL);
    destroy_publications();
}

/** Selecting a concrete value cannot hide a failing unselected definition. */
static void it_rejects_an_unselected_definition_cycle_before_connecting_the_consumer(void) {
    setup_publications("@!\nvalue\n/test/fixture/v1.0.0/src/other\n .good\n", "@...\n.good\n//integer\n.bad\n.bad\n");
    StaticOrdinaryType *type = ordinary("value");
    const StaticTerm *value = NULL;
    bool success = static_ordinary_resolve(&publications, type, &value);
    assert(!success && value == NULL && publications.status == STATIC_PUBLICATION_CONFLICT);
    assert(type->publication == NULL && guards.repeated_definition_ancestor);
    destroy_publications();
}

/** A simple ordinary generic path instantiates its omitted field parameters;
 * it is not identity with the residual constructor. Different fields can infer
 * different complete keys from later independent Type evidence.
 */
static void it_instantiates_generic_field_parameters_independently_before_later_anchors(void) {
    setup_publications("@!\nfirst\n/test/fixture/v1.0.0/src/other\nsecond\n/test/fixture/v1.0.0/src/other\nanchor&\n//integer\ntext anchor&\n//text\n",
        "@<\nvalue\n.slot\n@!\n");
    StaticOrdinaryType *fields[] = {ordinary("first"), ordinary("second")};
    const StaticModule *text = static_module_find(&inventory, project_find(&project, "//text"));
    const StaticTerm *text_type;
    bool success = static_term_nominal(&terms, text, NULL, 0, (SourceSpan){0}, &text_type);
    assert(success);
    const StaticTerm *arguments[] = {integer_type, text_type};
    for (size_t index = 0; index < 2; index++) {
        StaticResultOutcome outcome;
        success = static_ordinary_evaluate(&publications, fields[index], &outcome);
        if (!success) { fprintf(stderr, "Generic field inference: %s\n", publications.error.message); }
        assert(success);
        const StaticTerm *application = fields[index]->source->application->value;
        assert(application->kind == STATIC_TERM_NOMINAL && application->category == STATIC_CATEGORY_TYPE);
        assert(application->as.nominal.origin == target_module && application->as.nominal.residual_count == 0);
        const StaticTerm *hole = application->as.nominal.first_capture->value;
        assert(hole->kind == STATIC_TERM_VARIABLE && hole->as.variable.role == STATIC_VARIABLE_FIELD);
        assert(hole->as.variable.declaration == fields[index]->annotation->occurrence->statement);
        const StaticTerm *pending_value = NULL;
        StaticConstraintOutcome pending;
        success = static_constraint_value(&solver, fields[index]->value, &pending_value, &pending);
        assert(success && pending == STATIC_CONSTRAINT_PENDING && pending_value == NULL);
        StaticTermCapture slot = {.formal = application_formal(target_module, 0), .value = arguments[index]};
        const StaticTerm *expected, *resolved;
        success = static_term_nominal(&terms, target_module, &slot, 1, (SourceSpan){0}, &expected);
        assert(success);
        success = static_constraint_equate(&solver, fields[index]->value, expected, fields[index]->expression->span);
        assert(success);
        success = static_ordinary_resolve(&publications, fields[index], &resolved);
        assert(success && resolved == expected);
        assert(resolved->as.nominal.first_capture->value == arguments[index]);
    }
    assert(fields[0]->publication->value != fields[1]->publication->value);
    assert(target_constructor->as.nominal.residual_count == 1);
    destroy_publications();
}

/** Every measured prepare/export/publication allocation may refuse; no output
 * is published and the actual source/solver owners remain safely destructible.
 */
static void it_unwinds_every_ordinary_export_preparation_and_resolution_allocation_refusal(void) {
    for (size_t resolve = 0; resolve < 2; resolve++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_publications("@!\nvalue\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n//integer\n");
            const StaticResultContext *context = publication_context(caller_environment);
            StaticOrdinaryType *type = resolve ? ordinary("value") : NULL;
            const StaticTerm *value = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = resolve ? static_ordinary_resolve(&publications, type, &value) :
                static_ordinary_type(&publications, context, result_field("value"), &type);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && publications.status == STATIC_PUBLICATION_OUT_OF_MEMORY);
                assert(resolve ? value == NULL : type == NULL);
            }
            failure_at = 0;
            destroy_publications();
        }
    }
}

/** Exact registered handles, receivers and representable counters are enforced. */
static void it_refuses_copied_handles_missing_receivers_and_counter_overflow(void) {
    for (size_t kind = 0; kind < 8; kind++) {
        setup_publications("@!\nvalue\n/test/fixture/v1.0.0/src/other\n .out\nother value\n/test/fixture/v1.0.0/src/other\n .out\n", "@...\n.out\n//integer\n");
        const StaticResultContext *context = publication_context(caller_environment);
        StaticOrdinaryType *type = ordinary("value");
        StaticOrdinaryType copied = *type;
        const StaticTerm *value = NULL;
        StaticResultOutcome outcome = STATIC_RESULT_PENDING;
        bool success;
        if (kind == 0) { success = static_ordinary_resolve(&publications, &copied, &value); }
        else if (kind == 1) { success = static_ordinary_evaluate(&publications, type, NULL); }
        else if (kind == 2) { success = static_ordinary_evaluate(&publications, &copied, &outcome); }
        else if (kind == 3) { success = static_ordinary_type(&publications, context, result_field("value"), NULL); }
        else if (kind == 4) {
            publications.type_count = SIZE_MAX;
            /* Reuse still succeeds; overflow applies only to a new occurrence. */
            success = static_ordinary_type(&publications, context, result_field("value"), &type);
            assert(success && type != NULL);
            StaticOrdinaryType *saved = type;
            success = static_ordinary_type(&publications, context, result_field("other value"), &type);
            assert(!success && type == saved && publications.status == STATIC_PUBLICATION_OUT_OF_MEMORY);
            publications.type_count = 1;
            destroy_publications();
            continue;
        } else if (kind == 5) {
            AstTypeExpression expression = *type->expression;
            success = static_ordinary_type(&publications, context, &expression, &type);
        } else if (kind == 6) {
            StaticResultContext foreign = *context;
            success = static_ordinary_type(&publications, &foreign, type->expression, &type);
        } else { success = static_ordinary_resolve(&publications, type, NULL); }
        assert(!success && value == NULL && outcome == STATIC_RESULT_PENDING);
        assert(publications.status == STATIC_PUBLICATION_INVALID);
        destroy_publications();
    }
}

/** Run independent export, category, inference, ownership and identity partitions. */
static void run(void) {
    it_resolves_named_exports_as_concrete_types_without_producer_instances();
    it_refuses_open_suppliers_even_when_the_consumer_type_is_already_known();
    it_distinguishes_selected_type_outputs_from_other_template_outputs();
    it_rejects_unknown_and_nonpublic_export_names_without_adding_caller_symbols();
    it_keeps_normal_ordinary_type_inference_and_actual_source_provenance();
    it_refuses_substituted_marker_namespaces_without_publishing_a_value();
    it_rejects_an_unselected_definition_cycle_before_connecting_the_consumer();
    it_instantiates_generic_field_parameters_independently_before_later_anchors();
    it_unwinds_every_ordinary_export_preparation_and_resolution_allocation_refusal();
    it_refuses_copied_handles_missing_receivers_and_counter_overflow();
}
/** Counted teardown must finish before successful exit. */
int main(void) { run(); return 0; }
