/** @file
 * @brief Check actual common original augmentation, unchanged raw source and newly exposed conflicts.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
#include "../../../../../../tools/project/command/source/augmentation/augmentation.h"
/** Existing additional roots enter one common catalog; future absent roots do
 * not fabricate original inputs. Changed raw bytes and duplicate names refuse. */
static void it_captures_uncovered_existing_final_roots_in_one_original_domain(void) {
    setup_test(); assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/additional", 0700) == 0);
    file("generated/source/first", "@!\n"); file("generated/additional/second", "@!\n");
    SourceRootCapture capture; source_root_capture_init(&capture, sources.allocator);
    ModuleRoot root = {.path = "generated/source", .kind = MODULE_ROOT_STANDARD};
    assert(source_root_capture_build(&capture, &root, 1) == SOURCE_CAPTURE_OK);
    SourceTreeSelection tree; source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/source") == SOURCE_TREE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourcePhysicalPath destination; SourcePhysicalIssue issue;
    assert(source_physical_destination(&arena, "generated/copied", SOURCE_PHYSICAL_DIRECTORY, &destination, &issue) == SOURCE_PHYSICAL_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &tree, destination.path) == SOURCE_PLACEMENT_OK);
    ProjectSourceRootRequest extra = {.value = {.path = "generated/additional", .kind = MODULE_ROOT_STANDARD}};
    for (unsigned added = 0; added < 2; added++) {
        ProjectSourceRootSettings settings; project_source_root_settings_init(&settings, sources.allocator);
        assert(project_source_root_settings_build(&settings, &capture, &placement, SOURCE_OPERATION_COPY, added ? &extra : NULL, added) == PROJECT_SOURCE_ROOTS_OK);
        ProjectSourceAugmentation owner; project_source_augmentation_init(&owner, sources.allocator);
        assert(project_source_augmentation_build(&owner, &settings) == PROJECT_SOURCE_AUGMENTATION_OK && owner.valid);
        assert(owner.original->root_count == (added ? 2 : 1) && owner.original->view.count == (added ? 2 : 1));
        assert(owner.selected->capture == owner.original && owner.selected->count == 1);
        assert((owner.original == &capture) == !added);
        project_source_augmentation_destroy(&owner);
        if (added) {
            size_t total = 0, baseline = live_allocations;
            for (size_t failure = 0; !failure || failure <= total; failure++) {
                project_source_augmentation_init(&owner, sources.allocator); size_t before = allocation_count;
                if (failure) { failure_at = before + failure; }
                ProjectSourceAugmentationStatus status = project_source_augmentation_build(&owner, &settings);
                if (!failure) { assert(status == PROJECT_SOURCE_AUGMENTATION_OK); total = allocation_count - before; assert(total); }
                else { assert(status != PROJECT_SOURCE_AUGMENTATION_OK && !owner.valid); }
                failure_at = 0; project_source_augmentation_destroy(&owner); assert(live_allocations == baseline);
            }
            file("generated/source/first", "@!\n# changed\n");
            project_source_augmentation_init(&owner, sources.allocator);
            assert(project_source_augmentation_build(&owner, &settings) == PROJECT_SOURCE_AUGMENTATION_CHANGED);
            project_source_augmentation_destroy(&owner); file("generated/source/first", "@!\n");
            file("generated/additional/first", "@!\n");
            project_source_augmentation_init(&owner, sources.allocator);
            assert(project_source_augmentation_build(&owner, &settings) == PROJECT_SOURCE_AUGMENTATION_CAPTURE && !owner.valid);
            project_source_augmentation_destroy(&owner); assert(unlink("generated/additional/first") == 0);
        }
        project_source_root_settings_destroy(&settings);
    }
    source_tree_placement_destroy(&placement); source_arena_destroy(&arena);
    source_tree_selection_destroy(&tree); source_root_capture_destroy(&capture);
    assert(unlink("generated/source/first") == 0 && unlink("generated/additional/second") == 0
        && rmdir("generated/source") == 0 && rmdir("generated/additional") == 0); destroy_test();
}
/** Relaxing a final exclusion expands the same original root identity instead
 * of indexing that physical root twice and manufacturing duplicate definitions. */
static void it_expands_same_path_original_coverage_without_duplicate_root_indexing(void) {
    setup_test(); assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/source/hidden", 0700) == 0);
    file("generated/source/first", "@!\n"); file("generated/source/hidden/second", "@!\n");
    const char *excluded[] = {"hidden"};
    ModuleRoot root = {"generated/source", excluded, 1, MODULE_ROOT_STANDARD};
    SourceRootCapture capture; source_root_capture_init(&capture, sources.allocator);
    assert(source_root_capture_build(&capture, &root, 1) == SOURCE_CAPTURE_OK && capture.view.count == 1);
    SourceTreeSelection tree; source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/source/first") == SOURCE_TREE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourcePhysicalPath destination; SourcePhysicalIssue issue;
    assert(source_physical_destination(&arena, "generated/source/copied", SOURCE_PHYSICAL_ENDPOINT, &destination, &issue) == SOURCE_PHYSICAL_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &tree, destination.path) == SOURCE_PLACEMENT_OK);
    ProjectSourceRootRequest extra = {.value = {.path = "generated/source", .kind = MODULE_ROOT_STANDARD}};
    ProjectSourceRootSettings settings; project_source_root_settings_init(&settings, sources.allocator);
    assert(project_source_root_settings_build(&settings, &capture, &placement, SOURCE_OPERATION_COPY, &extra, 1) == PROJECT_SOURCE_ROOTS_OK);
    ProjectSourceAugmentation owner; project_source_augmentation_init(&owner, sources.allocator);
    assert(project_source_augmentation_build(&owner, &settings) == PROJECT_SOURCE_AUGMENTATION_OK && owner.valid);
    assert(owner.original != &capture && owner.original->root_count == 1 && owner.original->view.count == 2
        && owner.original->roots[0].excluded_count == 0 && capture.roots[0].excluded_count == 1);
    const ModuleSource *found = NULL;
    assert(module_catalog_find(&owner.original->catalog, "//hidden/second", &found) == MODULE_OK && found);
    assert(access(destination.path, F_OK) != 0 && errno == ENOENT);
    project_source_augmentation_destroy(&owner); project_source_root_settings_destroy(&settings);
    source_tree_placement_destroy(&placement); source_arena_destroy(&arena);
    source_tree_selection_destroy(&tree); source_root_capture_destroy(&capture);
    assert(unlink("generated/source/first") == 0 && unlink("generated/source/hidden/second") == 0
        && rmdir("generated/source/hidden") == 0 && rmdir("generated/source") == 0); destroy_test();
}
