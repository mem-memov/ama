/** @file
 * @brief Compose actual source applications with reuse, partial metadata and producer guard classification.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** No capability search can select a constructor for the unknown application. */
static const char *caller_source = "@...\n.partial\n.family\n.concrete partial\n/test/fixture/v1.0.0/src/other\n"
    ".unknown\n.family\n .x < //integer\n.reuse\n.type\n.anchor\n//integer\n";

/** Select original declaration/result syntax rather than foreign signature entries. */
static const AstTypeExpression *expression(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (!site->root || site->receiver != NULL || site->statement == NULL) { continue; }
        if (site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
        if (site->statement->kind == AST_STATIC_RESULT &&
            layout_text_is(site->statement->as.static_result.name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Unknown-interface T173 requests are re-normalized by the T177 adapter when
 * actual origin information arrives; one source-written RHS uses integer.
 */
static const StaticApplicationRecord *prepare(const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, const StaticTerm *head) {
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, expression, &requests));
    StaticTermOperand operand = {.request = requests->first, .value = integer_type};
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, environment, head,
        requests->input_count ? &operand : NULL, requests->input_count, &source));
    const StaticApplicationRecord *record;
    assert(static_application_source(&applications, source, &record));
    return record;
}

/** Unapplied constructors remain metadata, unknown applied heads remain Pending,
 * and plain supplied Types add no edge even when their nominal origin is unknown.
 */
static void it_distinguishes_partial_forwarding_unknown_application_and_plain_type_reuse(void) {
    setup_guards(caller_source, "@<\nargument\n.x\n@!\n");
    const StaticTerm *family = application_symbol(".family"), *type = application_symbol(".type");
    assert(static_constraint_describe(&solver, family, STATIC_CATEGORY_TEMPLATE,
        target_constructor->signature, expression(caller_module, ".partial")->span));
    assert(static_constraint_describe(&solver, type, STATIC_CATEGORY_TYPE, NULL,
        expression(caller_module, ".reuse")->span));
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const char *names[] = {".partial", ".concrete partial", ".unknown", ".reuse"};
    StaticCreationOutcome expected[] = {STATIC_CREATION_METADATA, STATIC_CREATION_METADATA,
        STATIC_CREATION_PENDING, STATIC_CREATION_REUSE};
    for (size_t partition = 0; partition < 4; partition++) {
        const StaticTerm *head = partition == 1 ? target_constructor : partition == 3 ? type : family;
        const StaticApplicationRecord *record = prepare(caller_environment,
            expression(caller_module, names[partition]), head);
        const StaticCreationPath *output = NULL;
        StaticCreationOutcome outcome;
        assert(static_creation_enter(&guards, root, record, &output, &outcome));
        assert(outcome == expected[partition] && output == NULL && guards.path_count == 1);
    }
    assert(caller_module->parameter_count == 2);
    destroy_guards();
}

/** Static graph producers enter through their source-backed compiler identities.
 * An ordinary Type application adapter cannot route or fabricate their origin.
 */
static void it_requires_static_producer_path_and_refuses_nominal_application_routing(void) {
    setup_guards("@...\n/test/fixture/v1.0.0/src/other\n .out > .received\n.anchor\n//integer\n",
        "@...\n.out\n//integer\n");
    const StaticCreationPath *root, *producer;
    assert(static_creation_root(&guards, caller_environment, &root));
    const AstTypeExpression *site = NULL;
    for (const StaticOccurrence *item = caller_module->first_occurrence; item; item = item->next) {
        if (item->origin == target_module->definition && item->receiver == NULL) { site = item->expression; break; }
    }
    assert(site && target_constructor == NULL);
    StaticCreationOutcome outcome;
    assert(static_creation_producer_enter(&guards, root, site, NULL, 0, &producer, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && producer->parent == root);
    assert(producer->vertex->key->definition == target_module && producer->vertex->value == NULL);
    assert(guards.path_count == 2 && guards.edge_count == 1);
    StaticApplicationContext context;
    assert(!static_application_context(&applications, caller_environment, site, &context));
    StaticGuardClosure closure;
    assert(!static_guard_refresh(&guards, &closure) && guards.status == STATIC_GUARD_INVALID);
    assert(guards.error.message == applications.error.message);
    destroy_guards();
}

/** Closed ordinary namespaces reuse their identity without a creation edge;
 * ordinary instance fields still retain their actual nominal creation use.
 */
static void it_reuses_fixed_ordinary_namespaces_without_creation_edges(void) {
    setup_guards("@!\nanchor&\n//integer\n/test/fixture/v1.0.0/src/other\n value >> copied\n/test/fixture/v1.0.0/src/other\n value >> copied again\nfield&\n/test/fixture/v1.0.0/src/other\n",
        "@=\nvalue\n//integer\n |0\n");
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const AstTypeExpression *namespaces[2] = {0}, *field = NULL;
    size_t count = 0;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (!site->root || site->receiver != NULL) { continue; }
        if (site->context == STATIC_CONTEXT_NAMESPACE) {
            assert(count < 2); namespaces[count++] = site->expression;
        } else if (site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, "field")) { field = site->expression; }
    }
    assert(count == 2 && field && target_module->parameter_count == 0);
    for (size_t index = 0; index < 2; index++) {
        const StaticApplicationRecord *record = prepare(caller_environment, namespaces[index], target_constructor);
        const StaticCreationPath *output = root;
        StaticCreationOutcome outcome;
        assert(static_creation_enter(&guards, root, record, &output, &outcome));
        assert(outcome == STATIC_CREATION_REUSE && output == root && guards.path_count == 1);
    }
    const StaticApplicationRecord *record = prepare(caller_environment, field, target_constructor);
    const StaticCreationPath *child;
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, root, record, &child, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && child->parent == root && guards.path_count == 2);
    destroy_guards();
}

/** Execute source/term/application handoffs for the current creation roles. */
static void run(void) {
    it_distinguishes_partial_forwarding_unknown_application_and_plain_type_reuse();
    it_requires_static_producer_path_and_refuses_nominal_application_routing();
    it_reuses_fixed_ordinary_namespaces_without_creation_edges();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
