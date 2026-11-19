/** @file
 * @brief Verify canonical nominal partitions, deep graphs and refused publication.
 */
#include "../../common.c"
#include "semantic/static/terms/store.h"

/** Complete values preserve original binding order, including unused captures;
 * partial constructors bind their residuals locally rather than inventing holes.
 */
static void it_canonicalizes_original_maps_and_preserves_symbolic_captures(void) {
    setup_static("@<\nfirst argument\n...first\nsecond argument\n...second\n@!\n", "@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *origin = static_module_find(&inventory, project.entry);
    const StaticModule *plain = static_module_find(&inventory, project_find(&project, ".other"));
    assert(origin->first->name.start == static_name_site(project.entry, "...first").start);
    StaticTermStore store;
    static_term_store_init(&store, &inventory, sources.allocator);
    const StaticTerm *base, *partial, *complete, *same, *symbol, *symbolic;
    const StaticTermEnvironment *generic, *applied;
    assert(static_term_nominal(&store, plain, NULL, 0, (SourceSpan){0}, &base));
    assert(static_term_environment(&store, origin, NULL, &generic));
    assert(static_term_symbol(&store, generic, origin->first, &symbol));
    StaticTermCapture captures[] = {{.formal = origin->first->next, .value = base},
        {.formal = origin->first, .value = base}};
    assert(static_term_nominal(&store, origin, captures, 1, (SourceSpan){0}, &partial));
    assert(partial->category == STATIC_CATEGORY_TEMPLATE && partial->ground);
    assert(partial->as.nominal.first_residual->formal == origin->first);
    assert(partial->signature->count == 1);
    assert(static_term_nominal(&store, origin, captures, 2, (SourceSpan){0}, &complete));
    StaticTermCapture ordered[] = {captures[1], captures[0]};
    assert(static_term_nominal(&store, origin, ordered, 2, origin->first->site, &same));
    assert(same == complete && complete->as.nominal.first_capture->formal == origin->first);
    assert(complete->category == STATIC_CATEGORY_TYPE && complete->ground);
    assert(static_term_environment(&store, origin, complete, &applied));
    assert(static_term_symbol(&store, applied, origin->first, &same) && same == base);
    ordered[0].value = symbol;
    assert(static_term_nominal(&store, origin, ordered, 2, (SourceSpan){0}, &symbolic));
    bool result = true;
    assert(static_term_ground(&store, symbolic, &result) && !result);
    assert(static_term_equal(&store, symbolic, complete, &result) && !result);
    assert(static_term_equal(&store, complete, complete, &result) && result);
    const StaticTerm *deep = base;
    for (size_t index = 0; index < 512; index++) {
        ordered[0].value = deep;
        assert(static_term_nominal(&store, origin, ordered, 2, (SourceSpan){0}, &deep));
        assert(deep->ground);
    }
    StaticTermCapture duplicate[] = {captures[0], captures[0]};
    same = deep;
    assert(!static_term_nominal(&store, origin, duplicate, 2, (SourceSpan){0}, &same));
    assert(same == deep && store.status == STATIC_TERM_INVALID && inventory.valid);
    static_term_store_destroy(&store);
    destroy_static();
}

/** Refuse each actual allocation while building both residual and fixed records;
 * prior values remain published and all partial arena records remain destructible.
 */
static void it_retains_prior_values_on_every_nominal_allocation_refusal(void) {
    for (size_t fixed = 0; fixed <= 1; fixed++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_static("@<\nfirst argument\n...first\nsecond argument\n...second\n@!\n", "@!\n");
            assert(static_inventory_collect(&inventory) == STATIC_OK);
            const StaticModule *origin = static_module_find(&inventory, project.entry);
            const StaticModule *plain = static_module_find(&inventory, project_find(&project, ".other"));
            StaticTermStore store;
            static_term_store_init(&store, &inventory, sources.allocator);
            const StaticTerm *base, *output;
            assert(static_term_nominal(&store, plain, NULL, 0, (SourceSpan){0}, &base));
            StaticTermCapture capture = {.formal = origin->first, .value = base};
            size_t before = allocation_count;
            if (failure != 0) { failure_at = before + failure; }
            output = base;
            bool success = static_term_nominal(&store, origin, &capture, fixed, (SourceSpan){0}, &output);
            if (failure == 0) { assert(success); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && output == base && store.status == STATIC_TERM_OUT_OF_MEMORY);
                assert(store.term_count == 1 && store.first == base && store.last == base);
                size_t after = allocation_count;
                assert(!static_term_nominal(&store, origin, NULL, 0, (SourceSpan){0}, &output));
                assert(allocation_count == after);
            }
            failure_at = 0;
            static_term_store_destroy(&store);
            destroy_static();
        }
    }
}

/** Run independent canonical identity and resource partitions. */
static void run(void) {
    it_canonicalizes_original_maps_and_preserves_symbolic_captures();
    it_retains_prior_values_on_every_nominal_allocation_refusal();
}
/** Return success after all assertions and counted owner teardown. */
int main(void) { run(); return 0; }
