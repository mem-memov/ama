/** @file
 * @brief Verify exact original/domain/mode guards and actual missing/open test refusal.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** No missing/open direct tests can be turned into a successful copy preparation. */
static void it_refuses_missing_and_only_open_required_tests_using_actual_compiler_admission(void) {
    for (unsigned missing = 0; missing < 2; missing++) {
        prepare_profile(false, !missing, missing != 0); select_main();
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_EXPANSION);
        assert(!copy.valid && !copy.closure.valid && copy.issue.message);
        assert(copy.expansion_status == (missing ? ACQUISITION_CLOSURE_PREPARATION_FAILED : ACQUISITION_CLOSURE_MISSING_TEST));
        assert((analysis.probes != NULL) == !missing); finish_copy();
    }
}
/** Ignored-version bytes never acquire the selected local environment. */
static void it_refuses_ignored_version_selection_before_structural_preparation_or_test_probes(void) {
    prepare();
    const char *directories[] = {"generated/project/dependencies", "generated/project/dependencies/vendor",
        "generated/project/dependencies/vendor/pkg", "generated/project/dependencies/vendor/pkg/v1.0.0",
        "generated/project/dependencies/vendor/pkg/v1.0.0/src"};
    for (size_t index = 0; index < sizeof(directories) / sizeof(*directories); index++) { directory(directories[index]); }
    file("generated/project/dependencies/vendor/pkg/v1.0.0/src/ignored", "\xff ignored opaque input\n");
    source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/project/dependencies/vendor/pkg/v1.0.0/src") == SOURCE_TREE_OK);
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_DOMAIN);
    assert(!copy.valid && !copy.closure.attempted && !copy.expansion.declarations_checked && !analysis.probes);
    assert(copy.domain_status == SOURCE_TREE_DOMAIN_UNSELECTED_VERSION && copy.failed_path); finish_copy();
}
/** Invalid modes/owners refuse without normalizing or recapturing another original view. */
static void it_refuses_invalid_copy_selection_and_mode_interfaces_without_provider_effects(void) {
    for (unsigned choice = 0; choice < 4; choice++) {
        prepare(); select_main(); SourceRootCapture foreign = capture;
        source_copy_closure_init(&copy, sources.allocator, choice == 0 ? &foreign : &capture, &analysis, NULL);
        if (choice == 1) { tree.valid = false; }
        assert(source_copy_closure_build(&copy, choice == 2 ? NULL : &tree,
            choice == 3 ? (SourceCopyDependencyMode)99 : SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_INVALID);
        assert(!copy.valid && !copy.closure.attempted && !analysis.probes); finish_copy();
    }
}
