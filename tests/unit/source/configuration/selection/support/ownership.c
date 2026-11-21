/**
 * @file
 * @brief Validate actual-view metadata and preserve outputs across all measured allocation refusals.
 * Contract: [selection](../../../../../../docs/implementation/project-configuration.md#t262-selection-and-explicit-edge-contract).
 */

/** Every failed root union or shared view remains owned and unpublished. */
static void it_validates_view_metadata_and_cleans_every_allocation_refusal(void) {
    prepare();
    DependencyViewInput bad[] = {inputs[VERSION], inputs[A], inputs[A]};
    const DependencyView *output = view;
    assert(dependency_view_build(&results, bad, 3, &output, &issue) == DEPENDENCY_SELECTION_INVALID && output == view);
    assert(dependency_view_build(&results, &inputs[A], 1, &output, &issue) == DEPENDENCY_SELECTION_INVALID);
    bad[0] = inputs[VERSION]; bad[1] = inputs[A]; bad[1].test_directory_prefix = 1;
    assert(dependency_view_build(&results, bad, 2, &output, &issue) == DEPENDENCY_SELECTION_INVALID);
    assert(dependency_view_build(&results, NULL, SIZE_MAX, &output, &issue) == DEPENDENCY_SELECTION_LIMIT);
    const char *wrong[] = {"", "/a", "a/", "a//b", "a+/wrong", "a++/b", "a/b+", "a.c"};
    for (size_t index = 0; index < sizeof(wrong) / sizeof(*wrong); index++) {
        DependencyViewInput projected = inputs[B];
        assert(dependency_view_provenance(inputs[B].identity, (const unsigned char *)wrong[index],
            strlen(wrong[index]), &projected, &issue) == DEPENDENCY_SELECTION_INVALID);
        assert(projected.identity == inputs[B].identity && projected.test_directory_prefix == 0);
    }
    const DependencySelector *patterns[] = {pattern("/ext/*")};
    const DependencySelection *selection = NULL;
    size_t before = allocation_count;
    assert(dependency_selection_build(&results, view, inputs[OWNER].identity, patterns, 1,
        &selection, &issue) == DEPENDENCY_SELECTION_OK);
    size_t allocations = allocation_count - before;
    for (size_t index = 1; index <= allocations; index++) {
        SourceArena temporary;
        source_arena_init(&temporary, sources.allocator);
        size_t live = live_allocations;
        const DependencySelection *unchanged = selection;
        failure_at = allocation_count + index;
        assert(dependency_selection_build(&temporary, view, inputs[OWNER].identity, patterns, 1,
            &unchanged, &issue) == DEPENDENCY_SELECTION_OUT_OF_MEMORY);
        failure_at = 0;
        assert(unchanged == selection);
        source_arena_destroy(&temporary);
        assert(live_allocations == live);
    }
    failure_at = allocation_count + 1;
    assert(dependency_view_build(&results, inputs, RECORDS, &output, &issue) == DEPENDENCY_SELECTION_OUT_OF_MEMORY);
    failure_at = 0;
    assert(output == view);
    finish();
}
