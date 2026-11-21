/**
 * @file
 * @brief Refuse every measured preparation/match allocation without changing published outputs.
 * Contract: [selector model](../../../../../../docs/implementation/project-configuration.md#t263-selector-matching-contract).
 */

/** Scratch is always retired; partial compilation remains arena-owned until explicit teardown. */
static void it_preserves_outputs_and_cleans_all_selector_allocation_refusals(void) {
    begin();
    SourceSpan expression = configuration_test_text("/acme/*/v1.0.0/src/a*");
    const DependencySelector *selector = NULL;
    size_t before = allocation_count;
    assert(dependency_selector_compile(&patterns, expression, NULL, &selector, &issue) == DEPENDENCY_SELECTOR_OK);
    size_t allocations = allocation_count - before;
    for (size_t index = 1; index <= allocations; index++) {
        SourceArena temporary;
        source_arena_init(&temporary, sources.allocator);
        const DependencySelector *output = selector;
        size_t live = live_allocations;
        failure_at = allocation_count + index;
        assert(dependency_selector_compile(&temporary, expression, NULL, &output, &issue) == DEPENDENCY_SELECTOR_OUT_OF_MEMORY);
        failure_at = 0;
        assert(output == selector && issue.span.source == expression.source);
        source_arena_destroy(&temporary);
        assert(live_allocations == live);
    }
    const char *identity = "/acme/pkg/v1.0.0/src/abc";
    before = allocation_count;
    assert(matches(selector, identity));
    allocations = allocation_count - before;
    for (size_t index = 1; index <= allocations; index++) {
        size_t live = live_allocations;
        bool output = false;
        failure_at = allocation_count + index;
        assert(dependency_selector_match(selector, (const unsigned char *)identity, strlen(identity),
            sources.allocator, &output, &issue) == DEPENDENCY_SELECTOR_OUT_OF_MEMORY);
        failure_at = 0;
        assert(!output && live_allocations == live);
    }
    failure_at = allocation_count + 1;
    assert(!matches(selector, "//item"));
    assert(allocation_count < failure_at);
    failure_at = 0;
    finish();
}
