/** @file
 * @brief Verify genuine raw-root carries, complete explicit final scopes and finite proposal failures.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
#include "../../../../../../tools/project/command/source/roots/roots.h"
/** Use real captured roots and raw directory placement, never fabricated ready
 * flags; final catalog/operation proof remains a separate consumer obligation. */
static void it_derives_root_settings_from_actual_raw_directory_carries_and_explicit_scopes(void) {
    setup_test(); assert(mkdir("generated/source", 0700) == 0); file("generated/source/main", "@!\n");
    SourceRootCapture capture; source_root_capture_init(&capture, sources.allocator);
    ModuleRoot root = {.path = "generated/source", .kind = MODULE_ROOT_STANDARD};
    assert(source_root_capture_build(&capture, &root, 1) == SOURCE_CAPTURE_OK);
    SourceTreeSelection tree; source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/source") == SOURCE_TREE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourcePhysicalPath destination; SourcePhysicalIssue issue;
    assert(source_physical_destination(&arena, "generated/copied", SOURCE_PHYSICAL_DIRECTORY, &destination, &issue) == SOURCE_PHYSICAL_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &tree, destination.path) == SOURCE_PLACEMENT_OK);
    for (unsigned move = 0; move < 2; move++) {
        ProjectSourceRootSettings owner; project_source_root_settings_init(&owner, sources.allocator);
        assert(project_source_root_settings_build(&owner, &capture, &placement, move ? SOURCE_OPERATION_MOVE : SOURCE_OPERATION_COPY,
            NULL, 0) == PROJECT_SOURCE_ROOTS_OK && owner.valid);
        assert(owner.final_count == (move ? 1 : 2) && owner.addition_count == (move ? 0 : 1));
        assert(owner.originals[0].action == (move ? SOURCE_OPERATION_ROOT_CARRY : SOURCE_OPERATION_ROOT_KEEP));
        assert(!strcmp(owner.final[move ? 0 : 1].path, destination.path));
        if (!move) { assert(owner.additions[0].source == 0 && owner.additions[0].final == 1); }
        project_source_root_settings_destroy(&owner);
    }
    const char *excluded[] = {"hidden"};
    ProjectSourceRootRequest extras[] = {{.value = {.path = "generated/source", .kind = MODULE_ROOT_STANDARD, .excluded = excluded, .excluded_count = 1}},
        {.value = {.path = "generated/new", .kind = MODULE_ROOT_STANDARD}}};
    ProjectSourceRootSettings owner; project_source_root_settings_init(&owner, sources.allocator);
    assert(project_source_root_settings_build(&owner, &capture, &placement, SOURCE_OPERATION_COPY, extras, 2) == PROJECT_SOURCE_ROOTS_OK);
    assert(owner.final_count == 3 && owner.addition_count == 2 && owner.final[0].excluded_count == 1
        && owner.final[0].excluded[0] == excluded[0] && owner.additions[1].source == SIZE_MAX);
    project_source_root_settings_destroy(&owner);
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        project_source_root_settings_init(&owner, sources.allocator); size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        ProjectSourceRootSettingsStatus status = project_source_root_settings_build(&owner, &capture, &placement, SOURCE_OPERATION_COPY, extras, 2);
        if (!failure) { assert(status == PROJECT_SOURCE_ROOTS_OK); total = allocation_count - before; assert(total); }
        else { assert(status != PROJECT_SOURCE_ROOTS_OK && !owner.valid); }
        failure_at = 0; project_source_root_settings_destroy(&owner); assert(live_allocations == baseline);
    }
    extras[0].value.kind = MODULE_ROOT_PROJECT;
    project_source_root_settings_init(&owner, sources.allocator);
    assert(project_source_root_settings_build(&owner, &capture, &placement, SOURCE_OPERATION_COPY, extras, 2) == PROJECT_SOURCE_ROOTS_KIND);
    project_source_root_settings_destroy(&owner);
    extras[0].value = (ModuleRoot){.path = "generated/./new", .kind = MODULE_ROOT_STANDARD};
    project_source_root_settings_init(&owner, sources.allocator);
    assert(project_source_root_settings_build(&owner, &capture, &placement, SOURCE_OPERATION_COPY, extras, 2) == PROJECT_SOURCE_ROOTS_CONFLICT);
    project_source_root_settings_destroy(&owner);
    assert(access(destination.path, F_OK) != 0 && errno == ENOENT);
    source_tree_placement_destroy(&placement); source_arena_destroy(&arena);
    source_tree_selection_destroy(&tree); source_root_capture_destroy(&capture);
    assert(unlink("generated/source/main") == 0 && rmdir("generated/source") == 0); destroy_test();
}
