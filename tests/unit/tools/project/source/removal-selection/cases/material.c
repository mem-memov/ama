/** @file
 * @brief Check exact raw and associated removal membership, missing tests, overlaps and changed observations.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** Exact removal sets include draft mains but neither topology ancestors nor unrelated siblings. */
static void it_builds_removal_sets_with_missing_tests_and_preserves_bare_directory_ancestors(void) {
    for (unsigned directory = 0; directory < 2; directory++) {
        begin_removal(MATERIAL_NONE, directory ? subtree_path : main_path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK && removal.valid);
        assert(removal.identity_count == (directory ? 1u : 2u));
        assert(!strcmp(removal.identities[removal.identity_count - 1], "/vendor/pkg/v1.0.0/src/parent/child"));
        if (!directory) { assert(!strcmp(removal.identities[0], "/vendor/pkg/v1.0.0/src/parent")); }
        SourceRemovalReview review = {0};
        assert(source_removal_selection_review(&removal, NULL, NULL, &review) == SOURCE_REMOVAL_OK && !review.consumers && !review.clients);
        size_t before = allocation_count;
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_INVALID && removal.valid);
        assert(allocation_count == before); end_removal(MATERIAL_NONE);
    }
}
/** Associated open helpers/data are removed; outside unused test, shared input and kept client all block. */
static void it_includes_actual_tests_and_reports_all_outside_graph_input_and_client_blockers(void) {
    begin_removal(MATERIAL_TESTS, main_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    assert(removal.identity_count == 4);
    const DeliverySourceDefinition *outside = definition("/vendor/pkg/v1.0.0/tests/outside");
    const DeliverySourceDefinition *child = definition("/vendor/pkg/v1.0.0/src/parent/child");
    const char *data = NULL;
    for (size_t index = 0; index < removal.path_count; index++) {
        if (strstr(removal.paths[index], "/data.bin")) { data = removal.paths[index]; }
        if (index) { assert(strcmp(removal.paths[index - 1], removal.paths[index]) < 0); }
    }
    assert(data);
    SourceRemovalInputUse input = {.owner = outside, .path = data};
    SourceClientRoot client = {.definition = child, .kind = SOURCE_CLIENT_EXPORT};
    SourceRemovalReview review = {0};
    assert(source_removal_selection_review(&removal, &input, &client, &review) == SOURCE_REMOVAL_BLOCKED);
    assert(review.consumers && review.consumers->owner == outside && review.consumers->kind == SOURCE_REMOVAL_MODULE);
    assert(review.consumers->next && review.consumers->next->kind == SOURCE_REMOVAL_INPUT && !review.consumers->next->next);
    assert(review.clients && review.clients->root == &client && !review.clients->next);
    client.action = SOURCE_CLIENT_MAP; client.destination = "/vendor/pkg/v1.0.0/src/new child";
    assert(source_removal_selection_review(&removal, &input, &client, &review) == SOURCE_REMOVAL_BLOCKED);
    assert(review.consumers && !review.clients); end_removal(MATERIAL_TESTS);
}
/** Removing the whole test-containing version drops internal edges but does not follow external dependencies. */
static void it_merges_overlapping_raw_and_associated_test_material_without_duplicate_keys(void) {
    begin_removal(MATERIAL_TESTS, "generated/project/packages/vendor/pkg/v1.0.0");
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    assert(removal.identity_count == 6);
    for (size_t index = 1; index < removal.path_count; index++) { assert(strcmp(removal.paths[index - 1], removal.paths[index]) < 0); }
    SourceRemovalReview review;
    assert(source_removal_selection_review(&removal, NULL, NULL, &review) == SOURCE_REMOVAL_OK); end_removal(MATERIAL_TESTS);
}
/** Missing original test mains and changed test bytes are contradictions, not optional missing tests. */
static void it_refuses_changed_associated_original_observations(void) {
    begin_removal(MATERIAL_TESTS, main_path);
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/check", "changed bytes\n");
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_CHANGED && !removal.valid);
    end_removal(MATERIAL_TESTS);
    begin_removal(MATERIAL_TESTS, main_path);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/check") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/helper") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/data.bin") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child") == 0);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_CHANGED && !removal.valid);
    test_material(); end_removal(MATERIAL_TESTS);
}
/** Plain deletion accepts support-only experiments and zero-module raw directories. */
static void it_accepts_support_only_tests_and_empty_directory_removal_without_executable_admission(void) {
    begin_removal(MATERIAL_OPAQUE, main_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    assert(removal.identity_count == 2);
    bool data = false;
    for (size_t index = 0; index < removal.path_count; index++) { if (strstr(removal.paths[index], "/data.bin")) { data = true; } }
    SourceRemovalReview review;
    assert(data && source_removal_selection_review(&removal, NULL, NULL, &review) == SOURCE_REMOVAL_OK);
    end_removal(MATERIAL_OPAQUE);
    begin_removal(MATERIAL_NONE, "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty");
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    assert(removal.identity_count == 0 && !removal.identities && removal.path_count == 1);
    assert(source_removal_selection_review(&removal, NULL, NULL, &review) == SOURCE_REMOVAL_OK);
    end_removal(MATERIAL_NONE);
}
