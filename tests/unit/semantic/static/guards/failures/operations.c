/** @file
 * @brief Prepare source-backed producer and definition guards before allocator cutpoints.
 */
#ifndef AMA_GUARD_FAILURE_OPERATIONS
#define AMA_GUARD_FAILURE_OPERATIONS
#include "../common.c"

/** Named graph inputs remain symbolic until independently anchored. */
static const char *caller_source = "@...\n...a\n...b\n...b\n..integer\n"
    "...t value\n...t\n...u value\n...u\n.other\n ...t < ...t\n ...u < ...u\n ...out > ...child\n...text\n..text\n";
static const char *target_source = "@...\n...out\n...t\n...u value\n...u\n...anchor\n..integer\n";

/** One operation is swept; its complete prepared inputs precede injection. */
typedef struct {
    const StaticTermEnvironment *environment;
    const StaticCreationPath *root;
    const AstTypeExpression *expression;
    StaticTermCapture inputs[2];
    const StaticDefinitionPath *definition;
} GuardOperation;

/** Exact scoped result identities remain distinct even when values agree. */
static const StaticSymbol *named(const char *name) {
    const StaticSymbol *symbol = static_symbol_find(caller_module, static_name_site(project.entry, name));
    assert(symbol); return symbol;
}

/** Applied producer scopes retain every original concrete input. */
static const StaticTermEnvironment *concrete_environment(void) {
    StaticTermCapture inputs[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, caller_module, inputs, 2, &environment));
    return environment;
}

/** Exact equations close existing scoped inputs without executing definitions. */
static void anchor(const StaticGuardOwner *owner) {
    assert(owner->value == NULL && owner->key->producer_applied);
    for (const StaticTermCapture *input = owner->key->producer_inputs; input; input = input->next) {
        assert(static_constraint_equate(&solver, input->value, integer_type, input->formal->site));
    }
}

/** Select the actual graph projection and resolve its named input values in
 * the lexical caller. This prepares no target scope and reads no output value.
 */
static void prepare_application(GuardOperation *operation) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->origin == target_module->definition) {
            operation->expression = site->expression; break;
        }
    }
    assert(operation->expression);
    for (size_t ordinal = 0; ordinal < 2; ordinal++) {
        operation->inputs[ordinal].formal = application_formal(target_module, ordinal);
        assert(static_term_symbol(&terms, operation->environment, application_formal(caller_module, ordinal),
            &operation->inputs[ordinal].value));
    }
}

/** Eight partitions cover generic/complete roots, symbolic/complete producer
 * entry, delayed closure and root/dependent static definitions.
 */
static GuardOperation prepare(size_t mode) {
    setup_guards(caller_source, target_source);
    GuardOperation operation = {.environment = caller_environment};
    if (mode == 1 || mode == 3 || mode == 7) { operation.environment = concrete_environment(); }
    if (mode < 2) { return operation; }
    assert(static_creation_root(&guards, operation.environment, &operation.root));
    if (mode <= 4) {
        prepare_application(&operation);
        if (mode == 4) {
            const StaticCreationPath *child;
            StaticCreationOutcome outcome;
            assert(static_creation_producer_enter(&guards, operation.root, operation.expression,
                operation.inputs, 2, &child, &outcome));
            assert(outcome == STATIC_CREATION_ENTERED);
            anchor(operation.root->owner);
            anchor(child->owner);
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

/** Execute only the measured operation and preserve output sentinels on
 * refusal, independently of retained destructible partial metadata.
 */
static bool execute(size_t mode, GuardOperation operation) {
    const StaticCreationPath *path = NULL;
    const StaticDefinitionPath *definition = NULL;
    StaticCreationOutcome creation = STATIC_CREATION_PENDING;
    StaticGuardClosure closure = STATIC_GUARD_PENDING;
    bool success;
    if (mode < 2) { success = static_creation_root(&guards, operation.environment, &path); }
    else if (mode < 4) {
        success = static_creation_producer_enter(&guards, operation.root, operation.expression,
            operation.inputs, 2, &path, &creation);
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
