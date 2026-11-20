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
    setup_constants("@=\nscalar\n.type\n |ignored literal bytes\nempty[]\n.type\n"
        "@<\ninput\n.type\n@>\noutput\n.result type\n@!\nanchor&\n//integer\ntarget&\n/test/fixture/v1.0.0/src/other\n", "@!\n");
}

/** Concrete ordinary owner inputs remain Type metadata, never Template arguments. */
static inline const StaticTermEnvironment *constant_applied_environment(void) {
    StaticTermCapture captures[] = {
        {.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTerm *owner;
    const StaticTermEnvironment *environment;
    bool success = static_term_nominal(&terms, caller_module, captures, 2, (SourceSpan){0}, &owner);
    assert(success);
    success = static_term_environment(&terms, caller_module, owner, &environment);
    assert(success);
    return environment;
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
