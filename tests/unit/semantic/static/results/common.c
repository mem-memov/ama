/** @file
 * @brief Own result preparation before its borrowed guards, applications and immutable sources.
 */
#ifndef AMA_STATIC_RESULT_TEST_COMMON
#define AMA_STATIC_RESULT_TEST_COMMON
#include "../guards/common.c"
#include "semantic/static/results/internal.h"

/** No result record may outlive its guard/value/source dependencies. */
static StaticResultPlan results;
/** Registry names and candidates outlive result records but not source owners. */
static IdentityRegistry result_identities;
static StaticRegistryPlan result_registry;

/** Initialize the actual canonical registry before the borrowing result owner. */
static inline void initialize_results(void) {
    identity_registry_init(&result_identities, sources.allocator);
    static_registry_init(&result_registry, &guards, &result_identities, sources.allocator);
    static_result_init(&results, &result_registry, sources.allocator);
}


/** Two named producer statements share one receiver; the first defines it.
 * A separate ordinary constructor supplies actual Template metadata.
 */
static const char *result_caller_source = "@...\n.first\n.type\n.partial\n.family\n"
    "/test/fixture/v1.0.0/src/other\n .out > .alias\n/test/fixture/v1.0.0/src/other\n .out > .alias\n.same\n.alias\n.anchor\n//integer\n.constructor anchor\n/test/fixture/v1.0.0/src/family\n";
static const char *result_target_source = "@...\n.out\n.slot\n";
/** An ordinary constructor is a Template value; a static producer is not. */
static const StaticTerm *result_family_constructor;

/** The guard fixture already proves no runtime setup occurs for these source records. */
static inline void setup_results(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/family", "@<\nvalue\n.slot\n@!\n");
    setup_guards(result_caller_source, result_target_source);
    const StaticModule *family = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/family"));
    assert(family && static_term_nominal(&terms, family, NULL, 0, (SourceSpan){0}, &result_family_constructor));
    size_t before = allocation_count;
    initialize_results();
    assert(results.status == STATIC_RESULT_OK && allocation_count == before);
}

/** Exact whole source names use the existing accepted symbol namespace. */
static inline const StaticSymbol *result_named(const StaticModule *module, const char *name) {
    const StaticSymbol *symbol = static_symbol_find(module, static_name_site(module->definition, name));
    assert(symbol); return symbol;
}

/** Only original declaration roots are selected; nested receivers are not heads. */
static inline const AstTypeExpression *result_field(const char *name) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Select an exact result-pair expression by its public source binder. */
static inline const AstTypeExpression *result_pair(const char *name) {
    const StaticSymbol *symbol = result_named(caller_module, name);
    assert(symbol->declaration && symbol->declaration->kind == AST_STATIC_RESULT);
    return symbol->declaration->as.static_result.expression;
}

/** Fixture statement ordinal is lexical source order, not a static argument
 * position. Only root named producer statements participate in this selector.
 */
static inline const AstTypeExpression *result_projection(size_t ordinal) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->receiver == NULL && site->root && site->statement->kind == AST_STATIC_PROJECTION &&
            site->statement->as.static_projection == site->expression) {
            if (ordinal-- == 0) { return site->expression; }
        }
    }
    assert(false); return NULL;
}

/** Applied compiler-only scope retains Type and Template arguments without
 * manufacturing an ordinary producer Type or runtime instance.
 */
static inline const StaticTermEnvironment *result_applied_environment(void) {
    StaticTermCapture captures[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = result_family_constructor}};
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, caller_module, captures, 2, &environment));
    return environment;
}

/** Destroy this arena first; final counted fixture teardown covers every refusal. */
static void destroy_results(void) {
    static_result_destroy(&results);
    assert(results.guards == NULL && results.first_source == NULL && results.first_definition == NULL);
    static_registry_destroy(&result_registry);
    identity_registry_destroy(&result_identities);
    destroy_guards();
}
#endif
