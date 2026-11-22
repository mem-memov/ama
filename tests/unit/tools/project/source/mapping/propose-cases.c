/** @file
 * @brief Verify captured main-to-placement composition without logical prefix substitution.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/propose.h"
/** Rename across package/version boundaries using the actual physical placement. */
static void it_derives_every_explicit_clone_from_its_placed_main_and_preserves_bare_tree_selection(void) {
    for (size_t choice = 0; choice < 3; choice++) {
        prepare();
        const char *operand = choice == 0 ? main_path : choice == 1 ? subtree_path
            : "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty";
        assert(source_tree_selection_read(&selection, &capture, operand) == SOURCE_TREE_OK);
        SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
        const char *destination = "/destination/packages/other/nested/pkg/v2.0.0-speed/src/copied";
        assert(source_tree_placement_build(&placement, &selection, destination) == SOURCE_PLACEMENT_OK);
        source_clone_map_init(&map, sources.allocator, &capture);
        ModuleRoot root = {"/destination", NULL, 0, MODULE_ROOT_PROJECT};
        SourceCloneTreeIssue issue = {0};
        assert(source_clone_map_propose_tree(&map, &placement, &root, 1, &issue) == SOURCE_CLONE_OK);
        assert(!map.ready && !issue.unit && !issue.path);
        size_t count = 0;
        for (const SourceCloneMapping *row = map.first; row; row = row->next) {
            count++;
            bool parent = row->original == original("/vendor/pkg/v1.0.0/src/parent");
            assert(!parent || choice == 0);
            assert(!strcmp(row->identity, parent ? "/other/nested/pkg/v2.0.0-speed/src/copied"
                : "/other/nested/pkg/v2.0.0-speed/src/copied/child"));
            assert(!strcmp(row->path, parent ? destination : choice == 0
                ? "/destination/packages/other/nested/pkg/v2.0.0-speed/src/copied+/child"
                : "/destination/packages/other/nested/pkg/v2.0.0-speed/src/copied/child"));
        }
        assert(count == (choice == 0 ? 2u : choice == 1 ? 1u : 0u));
        assert(source_clone_map_propose_tree(&map, &placement, &root, 1, &issue) == SOURCE_CLONE_OK);
        assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
        assert(source_clone_map_propose_tree(&map, &placement, &root, 1, &issue) == SOURCE_CLONE_INVALID);
        source_clone_map_destroy(&map); source_tree_placement_destroy(&placement); finish();
    }
}
/** Unrelated captures, invalid final namespaces and conflicting placements refuse explicitly. */
static void it_refuses_cross_capture_unplaced_or_conflicting_tree_proposals(void) {
    for (size_t choice = 0; choice < 5; choice++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
        SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
        assert(source_tree_placement_build(&placement, &selection, "/destination/src/copied") == SOURCE_PLACEMENT_OK);
        source_clone_map_init(&map, sources.allocator, &capture);
        ModuleRoot root = {"/destination/src", NULL, 0, MODULE_ROOT_STANDARD};
        SourceCloneTreeIssue issue = {0};
        if (choice == 0) { placement.ready = false; }
        if (choice == 1) { map.capture = NULL; }
        if (choice == 2) { root.kind = MODULE_ROOT_PROJECT; }
        if (choice == 3) {
            const SourceCloneMapping *row = NULL;
            assert(source_clone_map_add(&map, original("/vendor/pkg/v1.0.0/src/parent"), "/different", "//copied", &row) == SOURCE_CLONE_OK);
        }
        SourceCloneStatus status = source_clone_map_propose_tree(&map, &placement, &root, 1, choice == 4 ? NULL : &issue);
        assert(status == (choice == 3 ? SOURCE_CLONE_CONFLICT : SOURCE_CLONE_INVALID));
        assert(!map.ready && source_clone_map_seal(&map) == status);
        if (choice == 2) { assert(issue.unit && issue.path && issue.identity_status == SOURCE_IDENTITY_NOT_MODULE); }
        if (choice == 3) { assert(issue.unit && issue.path && map.conflict); }
        source_clone_map_destroy(&map); source_tree_placement_destroy(&placement); finish();
    }
}
/** Each joined identity/row/path allocation refuses without sealing or source mutation. */
static void it_retires_every_joined_tree_proposal_allocation_without_partial_acceptance(void) {
    size_t count = 0;
    for (size_t failure = 0; !failure || failure <= count; failure++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
        SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
        assert(source_tree_placement_build(&placement, &selection, "/destination/copied") == SOURCE_PLACEMENT_OK);
        source_clone_map_init(&map, sources.allocator, &capture);
        ModuleRoot root = {"/destination", NULL, 0, MODULE_ROOT_STANDARD};
        SourceCloneTreeIssue issue = {0}; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceCloneStatus status = source_clone_map_propose_tree(&map, &placement, &root, 1, &issue);
        if (!failure) { assert(status == SOURCE_CLONE_OK); count = allocation_count - before; assert(count); }
        else {
            assert(status == SOURCE_CLONE_MEMORY && !map.ready && issue.unit && issue.path);
            assert(source_clone_map_seal(&map) == SOURCE_CLONE_MEMORY && selection.valid && placement.ready);
        }
        failure_at = 0; source_clone_map_destroy(&map); source_tree_placement_destroy(&placement); finish();
    }
}
