/** @file
 * @brief Refuse every observed publication allocation and preserve original failures and immutable identities.
 */
#include "../common.c"

/** Every allocation of a real inferred producer/output publication may fail;
 * outputs stay untouched and partially checked descriptors remain destructible.
 * Include finite cyclic validation groups and their transitive closure scratch.
 */
static void it_unwinds_every_producer_closure_and_alias_publication_allocation_refusal(void) {
    for (size_t mode = 0; mode < 4; mode++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            if (mode == 3) {
                setup_publications("@...\n/test/fixture/v1.0.0/src/other\n .selected > .out\n",
                    "@...\n.selected\n//integer\n/test/fixture/v1.0.0/src/main\n .out > .unselected\n");
            } else {
                setup_publications(mode == 1 ?
                    "@...\n.expected\n//integer\n/test/fixture/v1.0.0/src/other\n .out > .expected\n" :
                    "@...\n/test/fixture/v1.0.0/src/other\n .slot < //integer\n .out > .alias\n", result_target_source);
            }
            const StaticResultContext *context = publication_context(caller_environment);
            const StaticPublication *publication = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = mode >= 2 ? static_publication_scope(&publications, context, &publication) :
                static_publication_resolve(&publications, context, result_projection(0), &publication);
            if (!failure) { assert(success); total = allocation_count - before; assert(total > 0); }
            else { assert(!success && publication == NULL && publications.status == STATIC_PUBLICATION_OUT_OF_MEMORY); }
            failure_at = 0;
            destroy_publications();
        }
    }
}

/** Invalid output/context handles and representable counter boundaries refuse
 * publication. Once failed, later calls preserve the original diagnostic.
 */
static void it_refuses_invalid_handles_counter_wrap_and_preserves_first_failures(void) {
    for (size_t mode = 0; mode < 4; mode++) {
        setup_publications("@...\n/test/fixture/v1.0.0/src/other\n .slot < //integer\n .out > .alias\n", result_target_source);
        const StaticResultContext *context = publication_context(caller_environment);
        StaticResultContext copied = *context;
        const StaticPublication *publication = NULL;
        if (mode == 2) { publications.count = SIZE_MAX; }
        if (mode == 3) { publications.alias_count = SIZE_MAX; }
        assert(!static_publication_resolve(&publications, mode == 1 ? &copied : context,
            result_projection(0), mode == 0 ? NULL : &publication));
        assert(publication == NULL && publications.status == (mode < 2 ? STATIC_PUBLICATION_INVALID : STATIC_PUBLICATION_OUT_OF_MEMORY));
        const char *message = publications.error.message;
        assert(!static_publication_resolve(&publications, context, result_projection(0), &publication));
        assert(publications.error.message == message);
        destroy_publications();
    }
}

/** Late contradictory equality evidence must be checked before returning an
 * already published descriptor. Prior immutable data remains unchanged.
 */
static void it_rechecks_equations_before_reusing_a_prior_publication(void) {
    setup_results();
    static_publication_init(&publications, &results, sources.allocator);
    const StaticTermEnvironment *environment = result_applied_environment();
    const StaticResultContext *context = publication_context(environment);
    const StaticResultDefinition *definition;
    assert(static_result_definition(&results, environment, result_named(caller_module, ".first"), &definition));
    const StaticPublication *publication;
    assert(static_publication_resolve(&publications, context, definition->source->expression, &publication));
    assert(publication->value == integer_type);
    assert(static_constraint_equate(&solver, definition->source->value, result_family_constructor, definition->symbol->site));
    const StaticPublication *output = NULL;
    assert(!static_publication_resolve(&publications, context, definition->source->expression, &output));
    assert(output == NULL && publications.status == STATIC_PUBLICATION_CONFLICT && publication->value == integer_type);
    destroy_publications();
}

/** Execute all observed resource refusals, invalid handles and late conflicts. */
static void run(void) {
    it_unwinds_every_producer_closure_and_alias_publication_allocation_refusal();
    it_refuses_invalid_handles_counter_wrap_and_preserves_first_failures();
    it_rechecks_equations_before_reusing_a_prior_publication();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
