/** @file
 * @brief Verify dependency mode, standard namespace, draft and empty-selection compositions.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Default/extend keep external units; clone mode closes ordinary cycles while keeping standard targets external. */
static void it_composes_every_copy_dependency_mode_with_actual_cycles_tests_and_standard_targets(void) {
    const SourceCopyDependencyMode modes[] = {SOURCE_COPY_KEEP_EXTERNAL, SOURCE_COPY_EXTEND_PACKAGE, SOURCE_COPY_CLONE_DEPENDENCIES};
    for (size_t choice = 0; choice < 3; choice++) {
        prepare_profile(true, false, false); select_main();
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, modes[choice]) == SOURCE_COPY_CLOSURE_OK && copy.valid);
        assert(copy.tree == &tree && copy.seed_count == 1 && !strcmp(copy.seeds[0], main_identity));
        assert(copy_role(main_identity) && !copy_role("//plain"));
        assert((copy_role("/vendor/pkg/v1.0.0/src/other") != NULL) == (choice == 2));
        size_t roles = 0, tests = 0;
        for (const AcquisitionRoleRecord *role = copy.closure.roles.records; role; role = role->next) { assert(role->expanded); roles++; }
        for (const AcquisitionTestRoot *test = copy.closure.tests; test; test = test->next) { tests++; }
        assert(roles == (choice == 2 ? 4u : 2u) && tests == (choice == 2 ? 2u : 1u));
        size_t before = allocation_count;
        assert(source_copy_closure_build(&copy, &tree, modes[choice]) == SOURCE_COPY_CLOSURE_INVALID && copy.valid);
        assert(allocation_count == before); finish_copy();
    }
}
/** Explicit standard selections participate structurally; drafts retain their descendants and context facts. */
static void it_prepares_explicit_standard_and_draft_tree_selections_without_invented_tests(void) {
    const char *operands[] = {"generated/standard/plain", "generated/project/packages/vendor/pkg/v1.0.0/src/parent"};
    for (unsigned choice = 0; choice < 2; choice++) {
        prepare(); source_tree_selection_init(&tree, sources.allocator);
        assert(source_tree_selection_read(&tree, &capture, operands[choice]) == SOURCE_TREE_OK);
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
        if (!choice) { assert(copy_role("//plain") && !copy.closure.tests && copy.seed_count == 1); }
        else {
            assert(copy_role("/vendor/pkg/v1.0.0/src/parent") && copy_role(child_identity));
            assert(copy.closure.tests && !copy.closure.tests->next && copy.seed_count == 2);
        }
        finish_copy();
    }
}
/** Empty raw directories still require admitted original proof, even with no provider callback. */
static void it_preserves_empty_raw_material_and_checks_original_ownership_before_empty_closure(void) {
    for (unsigned unrelated = 0; unrelated < 2; unrelated++) {
        prepare(); directory("generated/project/packages/vendor/pkg/v1.0.0/src/empty");
        source_tree_selection_init(&tree, sources.allocator);
        assert(source_tree_selection_read(&tree, &capture, "generated/project/packages/vendor/pkg/v1.0.0/src/empty") == SOURCE_TREE_OK);
        assert(!tree.count && tree.members && tree.members->directory);
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        const ModuleSnapshotEntry *saved = analysis.view.entries;
        if (unrelated) { analysis.view.entries = NULL; }
        SourceCopyClosureStatus status = source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL);
        analysis.view.entries = saved;
        if (unrelated) { assert(status == SOURCE_COPY_CLOSURE_PREPARATION && !copy.valid && !copy.closure.attempted); }
        else { assert(status == SOURCE_COPY_CLOSURE_OK && copy.valid && !copy.seeds && !copy.seed_count && !copy.closure.roles.records); }
        assert(!analysis.probes); finish_copy();
    }
}
