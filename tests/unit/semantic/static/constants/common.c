/** @file
 * @brief Own constant metadata before its borrowed publication, result and source fixtures.
 */
#ifndef AMA_STATIC_CONSTANT_TEST_COMMON
#define AMA_STATIC_CONSTANT_TEST_COMMON
#include "../publication/common.c"
#include "semantic/static/constants/internal.h"

/** Metadata does not own or initialize shared runtime values. */
static StaticConstantPlan constants;

/** Initialize without allocation after accepted source/interface collection. */
static void setup_constants(const char *caller, const char *target) {
    setup_publications(caller, target);
    size_t before = allocation_count;
    static_constant_init(&constants, &publications, sources.allocator);
    assert(constants.status == STATIC_CONSTANT_OK && allocation_count == before);
}

/** Explicit fixture root for the known ordinary target, independently of a read.
 * This models the required-type registry, never production name-based inference.
 */
static inline const StaticResultContext *constant_fixed_target_context(void) {
    assert(target_module->parameter_count == 0);
    const StaticTermEnvironment *environment;
    assert(static_term_environment(&terms, target_module, target_constructor, &environment));
    return publication_context(environment);
}

/** Preserve shared fields before interface text; collection must still bind
 * their later Type and Template parameters without adding implicit formals.
 */
static inline void setup_constant_fixture(void) {
    char source[2048];
    int length = snprintf(source, sizeof(source), "@=\nscalar\n...type\n |ignored literal bytes\nempty[]\n...type\n%s", result_caller_source);
    assert(length > 0 && (size_t)length < sizeof(source));
    setup_constants(source, result_target_source);
}

/** Select only a declared own shared field by its exact complete base name. */
static inline const AstNode *constant_named(const StaticModule *module, const char *name) {
    for (const AstNode *node = module->definition->syntax.constants.first; node; node = node->next) {
        if (node->kind == AST_DECLARATION && layout_text_is(node->as.declaration.name->name, name)) { return node; }
    }
    assert(false); return NULL;
}

/** Original namespace occurrences distinguish read statements from receiver
 * aliases and other declarations that reuse a type expression's owner node.
 */
static inline const AstNode *constant_namespace_read(size_t ordinal) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (site->root && site->receiver == NULL && site->context == STATIC_CONTEXT_NAMESPACE) {
            if (ordinal == 0) { return site->statement; }
            ordinal--;
        }
    }
    assert(false); return NULL;
}

/** Every source/allocator owner outlives constant descriptors; counted final
 * teardown proves failed partial metadata leaves no live allocations.
 */
static void destroy_constants(void) {
    static_constant_destroy(&constants);
    assert(constants.publications == NULL && constants.first_declaration == NULL);
    destroy_publications();
}
#endif
