/** @file
 * @brief Verify complete associated test projection, nested ownership and overlap failures.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/tests/tests.h"
/** Capture opaque/empty/plus material together with real concrete test mains. */
static void prepare_test_material(void) {
    setup_test(); fixtures();
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/main/input.bin", "opaque input\n");
    directory("generated/project/packages/vendor/pkg/v1.0.0/tests/main/empty");
    directory("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check+");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check+/helper", "@!\n");
    capture_fixtures(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
}
/** Extra files must not leak into the later independent closure profiles. */
static void finish_test_material(void) {
    finish_copy();
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/main/input.bin") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check+/helper") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check+") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/main/empty") == 0);
}
/** Produce a genuine complete main relation, including the nested test helper. */
static void test_main_map(SourceCloneMap *explicit_map, SourceCopyDestinations *destinations, SourceCloneMap *mains) {
    main_destination(explicit_map); source_copy_destinations_init(destinations, sources.allocator);
    assert(source_copy_destinations_build(destinations, &copy, explicit_map) == SOURCE_COPY_DESTINATIONS_OK && destinations->count == 5);
    const char *names[5];
    for (size_t i = 0; i < 5; i++) { names[i] = destinations->entries[i].identity; }
    source_clone_map_init(mains, sources.allocator, &capture);
    SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"}; SourceCopyMainsIssue issue;
    assert(source_copy_place_version_mains(mains, destinations, &version, names, 5, &issue) == SOURCE_COPY_MAINS_OK);
    assert(source_clone_map_seal(mains) == SOURCE_CLONE_OK);
}
/** Unit/test overlap agrees on concrete mains, while raw data/directories remain
 * complete; no automatic plus is added to mirrored source association names. */
static void it_maps_complete_test_trees_and_agrees_with_actual_unit_topology(void) {
    prepare_test_material(); SourceCloneMap explicit_map, mains; SourceCopyDestinations destinations;
    test_main_map(&explicit_map, &destinations, &mains);
    SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared/"};
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    SourceCopyUnitsIssue units; SourceCopyTestsIssue issue;
    assert(source_copy_map_unit_members(&files, &copy, &mains, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &issue) == SOURCE_COPY_TESTS_OK);
    bool data = false, empty = false, helper = false, root = false;
    for (const SourceFileMapping *row = files.first; row; row = row->next) {
        if (!strcmp(row->destination, "/prepared/tests/renamed/input.bin")) {
            data = true; assert(source_length(row->original->file.content) == 13);
            assert(!memcmp(source_bytes(row->original->file.content), "opaque input\n", 13));
        }
        if (!strcmp(row->destination, "/prepared/tests/renamed/empty")) { empty = true; assert(row->original->directory); }
        if (!strcmp(row->destination, "/prepared/tests/renamed")) { root = true; assert(row->original->directory); }
        if (!strcmp(row->destination, "/prepared/tests/renamed/check+/helper")) { helper = true; assert(!row->original->directory); }
    }
    assert(data && empty && helper && root);
    assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &issue) == SOURCE_COPY_TESTS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &issue) == SOURCE_COPY_TESTS_INVALID && files.ready);
    source_file_map_destroy(&files); source_clone_map_destroy(&mains); source_copy_destinations_destroy(&destinations);
    source_clone_map_destroy(&explicit_map); finish_test_material();
}
/** Missing correspondence/version, malformed version tables and conflicting unit
 * projections must refuse without becoming a complete material relation. */
static void it_refuses_test_mapping_authority_conflicts_and_all_allocation_failures(void) {
    prepare_test_material(); SourceCloneMap explicit_map, mains; SourceCopyDestinations destinations;
    test_main_map(&explicit_map, &destinations, &mains);
    SourceCloneMap empty; source_clone_map_init(&empty, sources.allocator, &capture);
    assert(source_clone_map_seal(&empty) == SOURCE_CLONE_OK);
    for (unsigned profile = 0; profile < 8; profile++) {
        SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyTestsIssue issue;
        SourceVersionPlacement versions[] = {{"/target/new/v3.0.0", "/prepared"}, {"/target/new/v3.0.0", "/other"}};
        size_t count = profile == 1 ? 0 : profile == 2 ? 2 : 1;
        if (profile == 3) { versions[0].identity = "//not a version"; }
        if (profile == 4) { versions[0].physical = "relative"; }
        if (profile == 5) {
            const AcquisitionMember *member = copy.closure.members.first;
            while (member && !strstr(member->path, "/tests/main/input.bin")) { member = member->next; } assert(member);
            const SourceFileMapping *row = NULL;
            assert(source_file_map_observation(&files, member->path, member->content, false, "/wrong", &row) == SOURCE_FILE_MAP_OK);
        }
        SourceCopyTestsStatus status = source_copy_map_test_members(&files, profile == 6 ? NULL : &copy,
            profile == 0 ? &empty : &mains, profile == 7 ? NULL : versions, count, &issue);
        assert(status == (profile == 0 ? SOURCE_COPY_TESTS_MAIN : profile == 5 ? SOURCE_COPY_TESTS_MAP
            : profile == 6 ? SOURCE_COPY_TESTS_INVALID : SOURCE_COPY_TESTS_VERSION));
        assert(source_file_map_seal(&files) != SOURCE_FILE_MAP_OK && !files.ready);
        if (profile == 5) { assert(files.status == SOURCE_FILE_MAP_CONFLICT && files.requester); }
        source_file_map_destroy(&files);
    }
    SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"};
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyTestsIssue issue;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyTestsStatus status = source_copy_map_test_members(&files, &copy, &mains, &version, 1, &issue);
        if (!failure) { assert(status == SOURCE_COPY_TESTS_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_COPY_TESTS_MEMORY && source_file_map_seal(&files) != SOURCE_FILE_MAP_OK); }
        failure_at = 0; source_file_map_destroy(&files); assert(live_allocations == baseline);
    }
    SourceFileMap invalid; source_file_map_init(&invalid, sources.allocator); SourceCopyTestsIssue issue;
    assert(source_copy_map_test_members(NULL, &copy, &mains, NULL, 0, &issue) == SOURCE_COPY_TESTS_INVALID);
    assert(source_copy_map_test_members(&invalid, &copy, &mains, NULL, 0, NULL) == SOURCE_COPY_TESTS_INVALID && invalid.status == SOURCE_FILE_MAP_OK);
    source_file_map_destroy(&invalid); source_clone_map_destroy(&empty); source_clone_map_destroy(&mains);
    source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map); finish_test_material();
}
/** A nested source branch may have a different explicit destination version.
 * Its test directory wins over the parent's enclosing capture, including the
 * nested root directory itself; no stale parent-relative copy may survive.
 */
static void it_maps_nested_test_material_by_deepest_source_association_across_versions(void) {
    setup_test(); fixtures();
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent", "@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/check", "@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/data.bin", "nested input\n");
    capture_fixtures(); source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/project/packages/vendor/pkg/v1.0.0/src") == SOURCE_TREE_OK);
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceCloneMap explicit_map; source_clone_map_init(&explicit_map, sources.allocator, &capture);
    const char *const original_names[] = {main_identity, "/vendor/pkg/v1.0.0/src/other",
        "/vendor/pkg/v1.0.0/src/parent", child_identity};
    const char *const final_names[] = {"/target/a/v2.0.0/src/main", "/target/b/v2.0.0/src/other",
        "/target/a/v2.0.0/src/outer", "/target/b/v2.0.0/src/inner"};
    const char *const final_paths[] = {"/prepared/a/src/main", "/prepared/b/src/other",
        "/prepared/a/src/outer", "/prepared/b/src/inner"};
    for (size_t i = 0; i < 4; i++) {
        const ModuleSource *source = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, original_names[i], &source) == MODULE_OK);
        assert(source_clone_map_add(&explicit_map, source, final_paths[i], final_names[i], &row) == SOURCE_CLONE_OK);
    }
    SourceCopyDestinations destinations; source_copy_destinations_init(&destinations, sources.allocator);
    assert(source_copy_destinations_build(&destinations, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK && destinations.count == 8);
    SourceVersionPlacement versions[] = {{"/target/a/v2.0.0", "/prepared/a"}, {"/target/b/v2.0.0", "/prepared/b"}};
    SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, &capture);
    for (size_t v = 0; v < 2; v++) {
        const char *names[8]; size_t count = 0;
        for (size_t i = 0; i < destinations.count; i++) {
            if (!strcmp(destinations.entries[i].version, versions[v].identity)) { names[count++] = destinations.entries[i].identity; }
        }
        SourceCopyMainsIssue issue;
        assert(source_copy_place_version_mains(&mains, &destinations, &versions[v], names, count, &issue) == SOURCE_COPY_MAINS_OK);
    }
    assert(source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        if (reverse) { SourceVersionPlacement temporary = versions[0]; versions[0] = versions[1]; versions[1] = temporary; }
        SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyTestsIssue issue;
        assert(source_copy_map_test_members(&files, &copy, &mains, versions, 2, &issue) == SOURCE_COPY_TESTS_OK);
        bool inner = false, outer = false, raw = false;
        for (const SourceFileMapping *row = files.first; row; row = row->next) {
            assert(!strstr(row->destination, "/tests/outer/child"));
            if (!strcmp(row->destination, "/prepared/b/tests/inner")) { inner = true; assert(row->original->directory); }
            if (!strcmp(row->destination, "/prepared/b/tests/inner/data.bin")) { raw = true; assert(!row->original->directory); }
            if (!strcmp(row->destination, "/prepared/a/tests/outer/check")) { outer = true; }
        }
        assert(inner && outer && raw && source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        source_file_map_destroy(&files);
    }
    source_clone_map_destroy(&mains); source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map); finish_copy();
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/check") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/data.bin") == 0);
}
