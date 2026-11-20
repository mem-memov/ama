/** @file
 * @brief Verify ordinary field inference, partial graph values and independent namespaces.
 */
#include "../common.c"
#include "semantic/static/guards/plan.h"

/** Ordinary interfaces retain only name/type pairs; every template path in a
 * declaration has its own stable inference identity, including the input field.
 */
static const char *caller_source = "@=\nshared\n/test/fixture/v1.0.0/src/other\n@<\nfirst argument\n.a\n"
    "second argument\n.b\ninput&\n/test/fixture/v1.0.0/src/other\n@>\nreturned&\n/test/fixture/v1.0.0/src/other\n"
    "@!\nanchor&\n//integer\nfirst&\n/test/fixture/v1.0.0/src/other\nsecond&\n/test/fixture/v1.0.0/src/other\n"
    "/test/fixture/v1.0.0/src/other\n constant >> namespace target\n.a\n zero >> known namespace target\n";
static const char *target_source = "@=\nconstant\n.a\n@<\nfirst argument\n.a\n"
    "second argument\n.b\n@>\nresult\n.a\n@!\n";

/** Select an actual independently concrete containing specialization. */
static const StaticTermEnvironment *owner_environment(void) {
    StaticTermCapture captures[] = {
        {.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}
    };
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &owner));
    assert(static_term_environment(&terms, caller_module, owner, &environment));
    return environment;
}

/** Locate the owning declaration rather than one of its nested/namespace uses. */
static const StaticOccurrence *declaration(const char *name) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->root && site->statement->kind == AST_DECLARATION
            && layout_text_is(site->statement->as.declaration.name->name, name)) { return site; }
    }
    assert(false); return NULL;
}

/** Shared, input, result and body storage use the same inference rule; different
 * owner/field keys never share holes, while repeated uses of one field do.
 */
static void it_infers_independent_shared_input_result_and_body_fields(void) {
    setup_applications(caller_source, target_source);
    const StaticTermEnvironment *environment = owner_environment();
    const char *names[] = {"shared", "input", "returned", "first", "second"};
    const StaticTerm *previous = NULL;
    const StaticApplicationRecord *last = NULL;
    for (size_t index = 0; index < 5; index++) {
        const StaticOccurrence *site = declaration(names[index]);
        StaticApplicationContext context;
        assert(static_application_context(&applications, environment, site->expression, &context));
        assert(context.policy == STATIC_OMIT_FIELD && context.declaration == site->statement);
        const StaticApplicationRecord *record, *repeat;
        assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &record));
        assert(record->value->category == STATIC_CATEGORY_TYPE && record->input_count == 2);
        const StaticTerm *hole = record->first->value;
        assert(record->first->generated && record->last->generated && hole != previous);
        assert(hole->as.variable.role == STATIC_VARIABLE_FIELD && hole->as.variable.declaration == site->statement);
        assert(hole->as.variable.environment == environment);
        assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &repeat));
        assert(repeat->value == record->value && repeat->first->value == hole);
        previous = hole;
        last = record;
        for (const StaticApplicationInput *input = record->first; input; input = input->next) {
            assert(static_constraint_equate(&solver, input->value, integer_type, context.site));
        }
    }
    assert(caller_module->parameter_count == 2 && static_constraint_finish(&solver));
    StaticGuardPlan guards;
    static_guard_init(&guards, &applications, sources.allocator);
    const StaticCreationPath *root, *child = NULL, *repeat = NULL;
    assert(static_creation_root(&guards, environment, &root));
    StaticCreationOutcome creation;
    assert(static_creation_enter(&guards, root, last, &child, &creation) && creation == STATIC_CREATION_ENTERED);
    assert(child != NULL && static_creation_enter(&guards, root, last, &repeat, &creation));
    assert(creation == STATIC_CREATION_ENTERED && repeat == child);
    static_guard_destroy(&guards);
    destroy_applications();
}

/** A named partial graph application retains its unfilled local formal, rather
 * than introducing a field hole or an extra free parameter of the graph.
 */
static void it_preserves_partial_static_result_definitions(void) {
    setup_applications("@...\n.family result\n/test/fixture/v1.0.0/src/other\n .a < //integer\n"
        ".integer type\n//integer\n", target_source);
    const StaticOccurrence *site = caller_module->first_occurrence;
    assert(site->statement->kind == AST_STATIC_RESULT);
    StaticApplicationContext context;
    assert(static_application_context(&applications, caller_environment, site->expression, &context));
    assert(context.policy == STATIC_OMIT_PARTIAL);
    StaticApplicationInput input = {.formal = application_formal(target_module, 0), .value = integer_type};
    const StaticApplicationRecord *record;
    assert(static_application_forward(&applications, target_constructor, &input, 1, context, &record));
    assert(record->value->category == STATIC_CATEGORY_TEMPLATE && record->input_count == 1);
    assert(!record->first->generated && record->value->as.nominal.residual_count == 1);
    assert(caller_module->parameter_count == 0);
    destroy_applications();
}

/** An open namespace cannot create inference holes; a Type supplied through an
 * independently concrete owner parameter is forwarded unchanged.
 */
static void it_requires_independently_closed_namespaces_without_field_inference(void) {
    for (size_t open = 0; open < 2; open++) {
        setup_applications(caller_source, target_source);
        const StaticTermEnvironment *environment = owner_environment();
        const StaticOccurrence *namespace = NULL;
        for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
            if (site->context == STATIC_CONTEXT_NAMESPACE
                && layout_text_is(site->expression->head, open ? "/test/fixture/v1.0.0/src/other" : ".a")) { namespace = site; break; }
        }
        assert(namespace != NULL);
        StaticApplicationContext context;
        assert(static_application_context(&applications, environment, namespace->expression, &context));
        assert(context.policy == STATIC_OMIT_NAMESPACE);
        const StaticApplicationRecord *record = NULL;
        bool success = static_application_forward(&applications, open ? target_constructor : integer_type,
            NULL, 0, context, &record);
        assert(success == !open);
        if (open) { assert(record == NULL && applications.status == STATIC_APPLY_CONFLICT); }
        else {
            assert(record->value == integer_type && record->input_count == 0);
            StaticGuardPlan guards;
            static_guard_init(&guards, &applications, sources.allocator);
            const StaticCreationPath *root;
            assert(static_creation_root(&guards, environment, &root));
            const StaticCreationPath *unchanged = root;
            StaticCreationOutcome creation;
            assert(static_creation_enter(&guards, root, record, &unchanged, &creation));
            assert(creation == STATIC_CREATION_REUSE && unchanged == root && guards.path_count == 1);
            static_guard_destroy(&guards);
        }
        destroy_applications();
    }
}

/** Valid graph/selector syntax belongs to separate compiler consumers and cannot
 * manufacture a nominal Type through this internal constructor interface.
 */
static void it_routes_producers_and_export_selectors_outside_nominal_application(void) {
    const char *sources[] = {
        "@...\n/test/fixture/v1.0.0/src/other\n .out > .alias\n.integer type\n//integer\n",
        "@!\nanchor&\n//integer\nfield\n/test/fixture/v1.0.0/src/other\n .out\n"
    };
    for (size_t selector = 0; selector < 2; selector++) {
        setup_applications(sources[selector], "@...\n.out\n.free\n");
        const StaticOccurrence *site = caller_module->first_occurrence;
        while (site != NULL && site->origin != project_find(&project, "/test/fixture/v1.0.0/src/other")) { site = site->next; }
        assert(site != NULL);
        StaticApplicationContext context = {.policy = STATIC_OMIT_NAMESPACE};
        assert(!static_application_context(&applications, caller_environment, site->expression, &context));
        assert(applications.status == STATIC_APPLY_INVALID && context.policy == STATIC_OMIT_NAMESPACE);
        destroy_applications();
    }
}

/** Explicitly run each source-context partition. */
static void run(void) {
    it_infers_independent_shared_input_result_and_body_fields();
    it_preserves_partial_static_result_definitions();
    it_requires_independently_closed_namespaces_without_field_inference();
    it_routes_producers_and_export_selectors_outside_nominal_application();
}
/** Return success after complete counted teardown. */
int main(void) { run(); return 0; }
