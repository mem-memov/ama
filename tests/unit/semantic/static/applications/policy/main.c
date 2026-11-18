/** @file
 * @brief Verify parsed omission contexts keep local binders distinct from field and producer holes.
 */
#include "../common.c"

/** Distinct declarations and nested expressions have real registered lexical
 * sites; template/native bodies and namespace members are never executed here.
 */
static const char *caller_source = "@=\nshared\n.other\n"
    "@<\ninput&\n.other\n ...a < ...a\n ...b < ...b\n"
    "@>\nreturned&\n.other\n...family result\n.other\n"
    "@!\nanchor&\n..integer\nfirst&\n.other\nsecond&\n.other\n"
    "nested&\n.other\n ...a < .other\n  ... >\n"
    ".other\n constant >> namespace target\n"
    ".other\n ...a < ..integer\n ...b < ..integer\n constant >> complete namespace target\n"
    ".other\n ... >\n constant >> producer namespace target\n";
static const char *target_source = "@=\nconstant\n...a\n@<\n...a\n...b\n"
    "@>\n...result\n...a\n@!\n";

/** Source formals a,b determine one concrete containing-owner key, independently
 * of any field or producer input subsequently inferred at its own lexical site.
 */
static const StaticTermEnvironment *owner_environment(void) {
    StaticTermCapture captures[2] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTerm *owner;
    assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &owner));
    const StaticTermEnvironment *environment;
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    return environment;
}

/** Select actual root declarations rather than output receiver occurrences. */
static const StaticOccurrence *declaration(const char *name) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site; }
    }
    assert(false);
    return NULL;
}

/** Namespace source order is independent from field declaration order. */
static const StaticOccurrence *namespace_site(size_t index) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->context == STATIC_CONTEXT_NAMESPACE) {
            if (index == 0) { return site; }
            index--;
        }
    }
    assert(false);
    return NULL;
}

/** Shared constants, runtime results and body fields get distinct deterministic
 * holes; they add no owner formal, and known typed facts can close all of them.
 */
static void it_infers_independent_shared_result_and_body_field_bindings(void) {
    setup_applications(caller_source, target_source);
    const StaticTermEnvironment *environment = owner_environment();
    const char *names[] = {"shared", "returned", "first", "second"};
    const StaticTerm *previous = NULL;
    for (size_t index = 0; index < 4; index++) {
        const StaticOccurrence *site = declaration(names[index]);
        StaticApplicationContext context;
        assert(static_application_context(&applications, environment, site->expression, &context));
        assert(context.policy == STATIC_OMIT_FIELD && context.declaration == site->statement);
        const StaticApplicationRecord *record;
        assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &record));
        assert(record->value->category == STATIC_CATEGORY_TYPE && record->value->as.nominal.capture_count == 2);
        assert(record->input_count == 2 && record->first->generated && record->last->generated);
        const StaticTerm *hole = record->first->value;
        assert(hole->as.variable.role == STATIC_VARIABLE_FIELD && hole->as.variable.environment == environment);
        assert(hole->as.variable.declaration == site->statement && hole != previous);
        previous = hole;
        const StaticApplicationRecord *repeat;
        assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &repeat));
        assert(repeat->value == record->value && repeat->first->value == hole);
        for (const StaticApplicationInput *input = record->first; input; input = input->next) {
            assert(static_constraint_equate(&solver, input->value, integer_type, context.site));
        }
    }
    assert(caller_module->parameter_count == 2 && static_constraint_finish(&solver));
    destroy_applications();
}

/** Static result definitions preserve their local residual binders rather than
 * introducing field holes or runtime producer instances.
 */
static void it_preserves_partial_static_result_definitions(void) {
    setup_applications(caller_source, target_source);
    const StaticTermEnvironment *environment = owner_environment();
    const StaticOccurrence *definition = NULL;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement && site->statement->kind == AST_STATIC_RESULT &&
            layout_text_is(site->statement->as.static_result.name, "...family result")) { definition = site; break; }
    }
    assert(definition);
    StaticApplicationContext context;
    assert(static_application_context(&applications, environment, definition->expression, &context));
    assert(context.policy == STATIC_OMIT_PARTIAL);
    size_t before = terms.term_count;
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &record));
    assert(record->value == target_constructor && record->input_count == 0 && terms.term_count == before);
    assert(record->value->category == STATIC_CATEGORY_TEMPLATE && record->value->as.nominal.residual_count == 2);
    destroy_applications();
}

/** Ordinary input and namespace roots cannot invent omitted arguments. A fully
 * explicit namespace remains independently closed before any member lookup.
 */
static void it_requires_explicit_input_and_namespace_coverage_without_implicit_holes(void) {
    for (size_t partition = 0; partition < 3; partition++) {
        setup_applications(caller_source, target_source);
        const StaticTermEnvironment *environment = owner_environment();
        const StaticOccurrence *site = partition == 0 ? declaration("input") : namespace_site(partition - 1);
        StaticApplicationContext context;
        assert(static_application_context(&applications, environment, site->expression, &context));
        assert(context.policy == (partition == 0 ? STATIC_OMIT_INPUT : STATIC_OMIT_NAMESPACE));
        StaticApplicationInput inputs[2] = {{.formal = application_formal(target_module, 0), .value = integer_type},
            {.formal = application_formal(target_module, 1), .value = integer_type}};
        const StaticApplicationRecord *record = NULL;
        size_t before = terms.term_count;
        bool success = static_application_forward(&applications, target_constructor, partition == 2 ? inputs : NULL,
            partition == 2 ? 2 : 0, context, &record);
        assert(success == (partition == 2));
        if (partition == 2) {
            assert(record->value->category == STATIC_CATEGORY_TYPE && record->input_count == 2);
            assert(static_constraint_finish(&solver));
        } else {
            assert(record == NULL && applications.status == STATIC_APPLY_CONFLICT && terms.term_count == before);
            assert(applications.error.related_count == 1 && strstr(applications.error.message, "coverage"));
        }
        destroy_applications();
    }
}

/** Any output skip overrides namespace/argument omission. Nested producer holes
 * differ from the containing field's holes even for the same original formal.
 */
static void it_applies_uniform_producer_inference_to_namespaces_and_nested_arguments(void) {
    setup_applications(caller_source, target_source);
    const StaticTermEnvironment *environment = owner_environment();
    const StaticOccurrence *namespace = namespace_site(2), *nested = declaration("nested");
    const AstTypeExpression *inner = nested->expression->first->input;
    StaticApplicationContext namespace_context, inner_context, outer_context;
    assert(static_application_context(&applications, environment, namespace->expression, &namespace_context));
    assert(static_application_context(&applications, environment, inner, &inner_context));
    assert(static_application_context(&applications, environment, nested->expression, &outer_context));
    assert(namespace_context.policy == STATIC_OMIT_PRODUCER && inner_context.policy == STATIC_OMIT_PRODUCER);
    assert(outer_context.policy == STATIC_OMIT_FIELD);
    const StaticApplicationRecord *namespace_record, *inner_record, *outer_record;
    assert(static_application_forward(&applications, target_constructor, NULL, 0, namespace_context, &namespace_record));
    assert(static_application_forward(&applications, target_constructor, NULL, 0, inner_context, &inner_record));
    assert(namespace_record->value->category == STATIC_CATEGORY_TYPE && namespace_record->input_count == 2);
    assert(inner_record->first->value != namespace_record->first->value);
    StaticApplicationInput input = {.formal = application_formal(target_module, 0), .value = inner_record->value};
    assert(static_application_forward(&applications, target_constructor, &input, 1, outer_context, &outer_record));
    const StaticTerm *inner_b = inner_record->last->value, *outer_b = outer_record->last->value;
    assert(inner_b->as.variable.symbol == outer_b->as.variable.symbol && inner_b != outer_b);
    assert(inner_b->as.variable.role == STATIC_VARIABLE_PRODUCER && outer_b->as.variable.role == STATIC_VARIABLE_FIELD);
    StaticConstraintOutcome outcome;
    assert(static_constraint_solve(&solver, &outcome) && outcome == STATIC_CONSTRAINT_PENDING);
    assert(caller_module->parameter_count == 2 && inventory.valid);
    destroy_applications();
}

/** Execute parsed source-policy and no-hidden-parameter partitions. */
static void run(void) {
    it_infers_independent_shared_result_and_body_field_bindings();
    it_preserves_partial_static_result_definitions();
    it_requires_explicit_input_and_namespace_coverage_without_implicit_holes();
    it_applies_uniform_producer_inference_to_namespaces_and_nested_arguments();
}
/** Return success after assertions and complete counted teardown. */
int main(void) { run(); return 0; }
