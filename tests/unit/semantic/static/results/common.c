/** @file
 * @brief Own result preparation before its borrowed guards, applications and immutable sources.
 */
#ifndef AMA_STATIC_RESULT_TEST_COMMON
#define AMA_STATIC_RESULT_TEST_COMMON
#include "../guards/common.c"
#include "semantic/static/results/internal.h"

/** No result record may outlive its guard/value/source dependencies. */
static StaticResultPlan results;

/** Two original projections use one alias; only its first receiver defines it. */
static const char *result_caller_source = "@<\n...type\n...family\n ...slot\n@>\n...first\n...type\n"
    "...partial\n...family\n@!\nanchor&\n..integer\nfirst&\n.other\n ...out > ...alias\n"
    "second&\n.other\n ...out > ...alias\nsame&\n...alias\n";
static const char *result_target_source = "@<\n...slot\n@>\n...out\n...slot\n@!\n";

/** The guard fixture already proves no runtime setup occurs for these source records. */
static inline void setup_results(void) {
    setup_guards(result_caller_source, result_target_source);
    size_t before = allocation_count;
    static_result_init(&results, &guards, sources.allocator);
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

/** Applied caller scope keeps Type and one-slot Template arguments distinct. */
static inline const StaticTermEnvironment *result_applied_environment(void) {
    StaticTermCapture captures[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = target_constructor}};
    const StaticTerm *value;
    const StaticTermEnvironment *environment;
    assert(static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &value));
    assert(static_term_environment(&terms, caller_module, value, &environment));
    return environment;
}

/** Destroy this arena first; final counted fixture teardown covers every refusal. */
static void destroy_results(void) {
    static_result_destroy(&results);
    assert(results.guards == NULL && results.first_source == NULL && results.first_definition == NULL);
    destroy_guards();
}
#endif
