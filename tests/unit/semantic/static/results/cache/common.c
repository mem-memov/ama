/** @file
 * @brief Prepare independently justified bare-formal definition equations for cache boundary tests.
 */
#ifndef AMA_STATIC_RESULT_CACHE_TEST_COMMON
#define AMA_STATIC_RESULT_CACHE_TEST_COMMON
#include "../common.c"

/** A selected caller scope retains its exact checked creation frame. */
static const StaticResultContext *cache_context(const StaticTermEnvironment *environment) {
    const StaticCreationPath *root;
    const StaticResultContext *context;
    assert(static_creation_root(&guards, environment, &root));
    assert(static_result_context(&results, root, &context));
    return context;
}

/** Bare received Type/Template forwarding has no application inputs or outputs:
 * its value is exactly its scoped head. These equations model that independent
 * simple law without claiming that the iterative evaluator already exists.
 */
static StaticResultDefinitionUse *checked_bare(const StaticResultContext *context, const char *name) {
    StaticResultDefinitionUse *definition;
    assert(static_result_definition_use(&results, context, result_named(caller_module, name), NULL,
        (SourceSpan){0}, &definition));
    StaticResultUse *rhs;
    assert(static_result_use(&results, context, definition->definition->source->expression, definition->path, &rhs));
    assert(rhs->source->expression->first == NULL && rhs->source->occurrence->symbol->role == STATIC_PARAMETER);
    assert(static_constraint_equate(&solver, rhs->source->value, rhs->source->head, rhs->source->expression->span));
    assert(static_constraint_equate(&solver, definition->definition->value, rhs->source->value, definition->definition->symbol->site));
    StaticConstraintOutcome outcome;
    const StaticTerm *value;
    assert(static_constraint_value(&solver, rhs->source->value, &value, &outcome) && outcome == STATIC_CONSTRAINT_COMPLETE);
    rhs->outcome = STATIC_RESULT_READY;
    rhs->closed = true;
    definition->rhs = rhs;
    definition->outcome = STATIC_RESULT_READY;
    return definition;
}

/** The Type and partial-family cases have independently complete owner arguments. */
static StaticResultDefinitionUse *setup_checked(const char *name) {
    setup_results();
    return checked_bare(cache_context(result_applied_environment()), name);
}
#endif
