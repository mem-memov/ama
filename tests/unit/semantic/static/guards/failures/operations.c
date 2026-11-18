/** @file
 * @brief Prepare exact source-backed guard operations before each independent allocator cutpoint.
 */
#ifndef AMA_GUARD_FAILURE_OPERATIONS
#define AMA_GUARD_FAILURE_OPERATIONS
#include "../common.c"

/** Actual result a depends on b; the child field supplies omitted typed inputs. */
static const char *caller_source = "@<\n...t\n...u\n@>\n...a\n...b\n...b\n..integer\n"
    "@!\nanchor&\n..integer\nchild&\n.other\n";
static const char *target_source = "@<\n...t\n...u\n@>\n...out\n..integer\n@!\n";

/** Only one operation is swept; its inputs and existing graph precede injection. */
typedef struct {
    const StaticTermEnvironment *environment;
    const StaticCreationPath *root;
    const StaticApplicationRecord *application;
    const StaticDefinitionPath *definition;
} GuardOperation;

/** Exact scoped static result names are independent of their shared value Type. */
static const StaticSymbol *named(const char *name) {
    const StaticSymbol *symbol = static_symbol_find(caller_module, static_name_site(project.entry, name), SIZE_MAX);
    assert(symbol); return symbol;
}

/** Selected applied owners use genuine complete Type arguments. */
static const StaticTermEnvironment *concrete_environment(void) {
    StaticTermCapture captures[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTerm *value;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &value));
    assert(static_term_environment(&terms, caller_module, value, &environment));
    return environment;
}

/** All supplied bindings here are Type variables or integer already; exact
 * equations close symbolic keys without any source/body execution.
 */
static void anchor(const StaticTerm *value) {
    for (const StaticTermCapture *capture = value->as.nominal.first_capture; capture; capture = capture->next) {
        assert(static_constraint_equate(&solver, capture->value, integer_type, capture->formal->site));
    }
}

/** Prepare the original FIELD omission descriptor from the selected owner. */
static const StaticApplicationRecord *application(const StaticTermEnvironment *environment) {
    const AstTypeExpression *expression = NULL;
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, "child")) { expression = site->expression; break; }
    }
    assert(expression);
    StaticApplicationContext context;
    const StaticApplicationRecord *record;
    assert(static_application_context(&applications, environment, expression, &context));
    assert(static_application_forward(&applications, target_constructor, NULL, 0, context, &record));
    return record;
}

/** Eight public operation partitions include generic/complete roots, symbolic/
 * complete applications, delayed closure and root/dependent static definitions.
 */
static GuardOperation prepare(size_t mode) {
    setup_guards(caller_source, target_source);
    GuardOperation operation = {.environment = caller_environment};
    if (mode == 1 || mode == 3 || mode == 7) { operation.environment = concrete_environment(); }
    if (mode < 2) { return operation; }
    assert(static_creation_root(&guards, operation.environment, &operation.root));
    if (mode <= 4) {
        operation.application = application(operation.environment);
        if (mode == 3) { anchor(operation.application->value); }
        if (mode == 4) {
            const StaticCreationPath *child;
            StaticCreationOutcome outcome;
            assert(static_creation_enter(&guards, operation.root, operation.application, &child, &outcome));
            anchor(operation.root->owner->value);
            anchor(child->owner->value);
        }
        if (mode >= 3) {
            StaticConstraintOutcome outcome;
            assert(static_constraint_solve(&solver, &outcome));
        }
    }
    if (mode == 6) {
        assert(static_definition_enter(&guards, NULL, operation.root, named("...a"),
            (SourceSpan){0}, &operation.definition));
    }
    return operation;
}

/** Execute just the measured operation and verify untouched output sentinels
 * on failure, independently of any internally retained valid partial records.
 */
static bool execute(size_t mode, GuardOperation operation) {
    const StaticCreationPath *path = NULL;
    const StaticDefinitionPath *definition = NULL;
    StaticCreationOutcome creation = STATIC_CREATION_PENDING;
    StaticGuardClosure closure = STATIC_GUARD_PENDING;
    bool success;
    if (mode < 2) { success = static_creation_root(&guards, operation.environment, &path); }
    else if (mode < 4) {
        success = static_creation_enter(&guards, operation.root, operation.application, &path, &creation);
    } else if (mode == 4) { success = static_guard_refresh(&guards, &closure); }
    else {
        const StaticSymbol *target = named(mode == 6 ? "...b" : "...a");
        SourceSpan site = named("...a")->declaration->as.static_result.expression->span;
        success = static_definition_enter(&guards, operation.definition, operation.root, target, site, &definition);
    }
    if (!success) {
        assert(path == NULL && definition == NULL && creation == STATIC_CREATION_PENDING && closure == STATIC_GUARD_PENDING);
    } else if (mode == 4) { assert(closure == STATIC_GUARD_CLOSED); }
    else if (mode < 4) { assert(path); }
    else { assert(definition); }
    return success;
}
#endif
