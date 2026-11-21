/** @file
 * @brief Build actual producer-to-ordinary paths with separate source-only annotation predecessors.
 */
#include "../../../results/common.c"

/** One ordinary input is forwarded to a static factory selecting a branch Type. */
static const char *pending_caller = "@<\nargument\n.type\n@!\nvalue&\n/test/fixture/v1.0.0/src/other\n .slot < .type\n .out\nanchor&\n//integer\nother&\n//text\n";
static const char *pending_target = "@...\n.out\n/test/fixture/v1.0.0/src/branch\n .type < .slot\n";
static const char *pending_branch = "@<\nargument\n.type\n@!\nvalue&\n/test/fixture/v1.0.0/src/other\n .slot < .type\n .out\nvia&\n/test/fixture/v1.0.0/src/middle\n .slot < .type\n .out\n";

/** Preserve the exact scoped unknown, original branch annotation and first
 * actual factory/branch paths. These values borrow their fixture owners.
 */
typedef struct {
    const StaticTerm *input;
    const StaticCreationPath *factory;
    const StaticCreationPath *branch;
    const StaticCreationPath *annotation;
    const AstTypeExpression *expression;
} PendingFixture;

/** Locate one original declaration in the requested lexical module; source
 * spellings in other modules never stand for this identity.
 */
static const AstTypeExpression *pending_field(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (site->root && !site->receiver && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Real result evaluation creates the ordinary branch path under a producer,
 * but does not expand its runtime body or complete the unknown input key.
 * A separate annotation frame uses that branch's exact symbolic environment.
 */
static PendingFixture setup_pending(bool closed_branch) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/branch", pending_branch);
    file("generated/static scope/packages/test/fixture/v1.0.0/src/middle",
        "@...\n/test/fixture/v1.0.0/src/other\n .slot < .slot\n .out > .out\n");
    setup_guards(pending_caller, closed_branch ?
        "@...\n.out\n/test/fixture/v1.0.0/src/branch\n .type < //integer\n.unused\n.slot\n" : pending_target);
    initialize_results();
    PendingFixture fixture = {.input = application_symbol(".type")};
    const StaticCreationPath *annotation;
    const AstTypeExpression *expression = result_field("value");
    assert(static_creation_annotation(&guards, caller_environment, expression, &annotation));
    StaticTermCapture input = {.formal = application_formal(target_module, 0), .value = fixture.input};
    StaticCreationOutcome entered;
    assert(static_creation_producer_enter(&guards, annotation, expression, &input, 1, &fixture.factory, &entered));
    assert(entered == STATIC_CREATION_ENTERED);
    const StaticResultContext *context;
    assert(static_result_context(&results, fixture.factory, &context));
    const AstTypeExpression *result = result_named(target_module, ".out")->declaration->as.static_result.expression;
    StaticResultOutcome outcome;
    assert(static_result_evaluate(&results, context, result, &outcome));
    StaticResultUse *use;
    assert(static_result_use(&results, context, result, NULL, &use));
    assert(use->producer);
    assert(outcome == STATIC_RESULT_READY && use->closed == closed_branch);
    assert(use->producer->registry_use->key_state ==
        (closed_branch ? STATIC_REGISTRY_KEY_CLOSED : STATIC_REGISTRY_KEY_PENDING));
    fixture.branch = use->producer->creation;
    assert(fixture.branch->parent == fixture.factory);
    const StaticTermEnvironment *environment = fixture.branch->owner->environment;
    fixture.expression = pending_field(environment->definition, "value");
    assert(static_creation_annotation(&guards, environment, fixture.expression, &fixture.annotation));
    assert(fixture.annotation->owner == fixture.branch->owner && !fixture.annotation->parent);
    return fixture;
}

/** Submit the exact original branch producer use with a chosen full input map. */
static bool pending_enter(const StaticCreationPath *parent, const AstTypeExpression *expression,
    const StaticTerm *value, const StaticCreationPath **output, StaticCreationOutcome *outcome) {
    StaticTermCapture input = {.formal = application_formal(target_module, 0), .value = value};
    return static_creation_producer_enter(&guards, parent, expression, &input, 1, output, outcome);
}
