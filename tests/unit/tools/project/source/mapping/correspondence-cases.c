/** @file
 * @brief Check physical and nominal map agreement on real indexed mains, drafts and opaque inputs.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/correspondence.h"
/** Missing rows in either relation and contradictory paths/bytes/shapes cannot pass the join. */
static void it_requires_both_maps_to_cover_exactly_the_same_original_mains_and_observations(void) {
    for (unsigned choice = 0; choice < 6; choice++) {
        prepare(); assert(source_tree_selection_read(&selection, &capture, main_path) == SOURCE_TREE_OK);
        SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
        assert(source_tree_placement_build(&placement, &selection, "/final/packages/vendor/target/v2.0.0/src/copy") == SOURCE_PLACEMENT_OK);
        const ModuleSource *parent = original("/vendor/pkg/v1.0.0/src/parent");
        source_clone_map_init(&map, sources.allocator, &capture);
        ModuleRoot root = {"/final", NULL, 0, MODULE_ROOT_PROJECT};
        if (choice != 1) {
            SourceCloneTreeIssue proposal_issue = {0};
            assert(source_clone_map_propose_tree(&map, &placement, &root, 1, &proposal_issue) == SOURCE_CLONE_OK);
        } else {
            const ModuleSource *child = original("/vendor/pkg/v1.0.0/src/parent/child");
            const SourceCloneMapping *row = NULL;
            assert(source_clone_map_add(&map, child, "/final/packages/vendor/target/v2.0.0/src/copy+/child", "/vendor/target/v2.0.0/src/copy/child", &row) == SOURCE_CLONE_OK);
        }
        assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
        SourceFileMap files; source_file_map_init(&files, sources.allocator);
        AcquisitionUnitEntry modified = {0};
        for (const SourceTreePlacementEntry *entry = placement.entries; entry; entry = entry->next) {
            bool main = !strcmp(entry->original->file.path, source_path(parent->text));
            if (choice == 2 && main) { continue; }
            const AcquisitionUnitEntry *input = entry->original;
            if (main && (choice == 4 || choice == 5)) {
                modified = *input;
                if (choice == 4) { modified.file.content = original("/vendor/pkg/v1.0.0/src/parent/child")->text; }
                else { modified.directory = true; modified.file.content = NULL; }
                input = &modified;
            }
            const SourceFileMapping *row = NULL;
            assert(source_file_map_add(&files, input, choice == 3 && main ? "/other/copy" : entry->path, &row) == SOURCE_FILE_MAP_OK);
        }
        assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        SourceCorrespondenceIssue issue = {0};
        SourceCorrespondenceStatus expected = choice == 0 ? SOURCE_CORRESPONDENCE_OK
            : choice < 3 ? SOURCE_CORRESPONDENCE_MISSING : SOURCE_CORRESPONDENCE_CHANGED;
        assert(source_mapping_correspondence(&map, &files, &issue) == expected);
        if (choice == 0) { assert(!issue.unit && !issue.clone && !issue.file); }
        else { assert(issue.unit && issue.unit->source == parent && (issue.clone || issue.file)); }
        source_file_map_destroy(&files); source_clone_map_destroy(&map);
        source_tree_placement_destroy(&placement); finish();
    }
}
/** A sealed pair may be empty, while unsealed metadata is never a complete map proof. */
static void it_checks_mapping_correspondence_only_for_complete_relations_without_allocating(void) {
    prepare(); source_clone_map_init(&map, sources.allocator, &capture);
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    SourceCorrespondenceIssue issue = {0};
    assert(source_mapping_correspondence(&map, &files, &issue) == SOURCE_CORRESPONDENCE_INVALID);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    assert(source_mapping_correspondence(&map, &files, &issue) == SOURCE_CORRESPONDENCE_INVALID);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    size_t before = allocation_count; failure_at = before + 1;
    assert(source_mapping_correspondence(&map, &files, &issue) == SOURCE_CORRESPONDENCE_OK);
    assert(allocation_count == before && !issue.unit && !issue.clone && !issue.file);
    assert(source_mapping_correspondence(NULL, &files, &issue) == SOURCE_CORRESPONDENCE_INVALID);
    assert(source_mapping_correspondence(&map, NULL, &issue) == SOURCE_CORRESPONDENCE_INVALID);
    assert(source_mapping_correspondence(&map, &files, NULL) == SOURCE_CORRESPONDENCE_INVALID);
    failure_at = 0; source_file_map_destroy(&files); source_clone_map_destroy(&map); finish();
}
