/** @file
 * @brief Check actual copy placement agreement, complete namespace topology and failed-prefix sealing.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Topology derives from final names, including retained test anchors; physical
 * P disagreement, incomplete/duplicate indexes and conflicting C proposals refuse. */
static void it_checks_copy_main_placement_topology_explicit_agreement_and_failure_sealing(void) {
    prepare(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceCloneMap explicit_map; main_destination(&explicit_map);
    SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
    assert(source_copy_destinations_build(&result, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK);
    assert(result.count > 1 && result.count < 8);
    for (unsigned profile = 0; profile < 11; profile++) {
        SourceCloneMap map; source_clone_map_init(&map, sources.allocator, &capture);
        const char *names[9]; size_t count = result.count;
        for (size_t i = 0; i < count; i++) { names[i] = result.entries[i].identity; }
        SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"};
        SourceCopyMainsStatus expected = SOURCE_COPY_MAINS_INVALID;
        if (profile == 0) { count--; expected = SOURCE_COPY_MAINS_TOPOLOGY; }
        if (profile == 1) { names[count++] = names[0]; expected = SOURCE_COPY_MAINS_TOPOLOGY; }
        if (profile == 2) { version.physical = "/wrong"; expected = SOURCE_COPY_MAINS_EXPLICIT; }
        if (profile == 3) {
            const ModuleSource *different = NULL;
            for (size_t i = 0; i < result.count; i++) { if (result.entries[i].source != tree.units[0]->source) { different = result.entries[i].source; break; } }
            const SourceCloneMapping *row = NULL; assert(different);
            assert(source_clone_map_add(&map, different, "/prepared/src/renamed", "/target/new/v3.0.0/src/renamed", &row) == SOURCE_CLONE_OK);
            expected = SOURCE_COPY_MAINS_MAP;
        }
        if (profile == 4) { map.capture = NULL; }
        if (profile == 5) { assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK); }
        if (profile == 6) { names[count++] = "/target/new/v3.0.0/tests/renamed"; expected = SOURCE_COPY_MAINS_OK; }
        if (profile == 7) { version.identity = "/target/unused/v1.0.0"; expected = SOURCE_COPY_MAINS_OK; }
        if (profile == 9) { version.identity = "not a version"; }
        SourceCopyMainsIssue issue;
        assert(source_copy_place_version_mains(&map, profile == 10 ? NULL : &result,
            &version, profile == 8 ? NULL : names, count, &issue) == expected);
        if (expected == SOURCE_COPY_MAINS_OK) {
            assert(!map.ready);
            if (profile == 7) { assert(!map.first); }
            else {
                const SourceCloneMapping *row = map.first;
                while (row && strcmp(row->identity, "/target/new/v3.0.0/tests/renamed/check")) { row = row->next; }
                assert(row && !strcmp(row->path, "/prepared/tests/renamed+/check"));
            }
        } else if (profile != 5) { assert(source_clone_map_seal(&map) != SOURCE_CLONE_OK && !map.ready); }
        else { assert(map.ready && map.status == SOURCE_CLONE_OK); }
        source_clone_map_destroy(&map);
    }
    SourceCopyMainsIssue issue; SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"};
    assert(source_copy_place_version_mains(&explicit_map, &result, &version, NULL, 0, &issue) == SOURCE_COPY_MAINS_INVALID);
    assert(explicit_map.status == SOURCE_CLONE_OK && !explicit_map.ready);
    assert(source_copy_place_version_mains(NULL, &result, &version, NULL, 0, &issue) == SOURCE_COPY_MAINS_INVALID);
    assert(source_copy_place_version_mains(&explicit_map, &result, &version, NULL, 0, NULL) == SOURCE_COPY_MAINS_INVALID);
    source_copy_destinations_destroy(&result); source_clone_map_destroy(&explicit_map); finish_copy();
}
/** Every path/clone-row allocation failure poisons the partial map and retires
 * to the stable original/copy/logical-owner baseline without re-running analysis. */
static void it_retires_every_actual_copy_main_placement_allocation_failure(void) {
    prepare(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceCloneMap explicit_map; main_destination(&explicit_map);
    SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
    assert(source_copy_destinations_build(&result, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK);
    const char *names[8]; assert(result.count < 8);
    for (size_t i = 0; i < result.count; i++) { names[i] = result.entries[i].identity; }
    SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"};
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceCloneMap map; source_clone_map_init(&map, sources.allocator, &capture);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyMainsIssue issue;
        SourceCopyMainsStatus status = source_copy_place_version_mains(&map, &result, &version, names, result.count, &issue);
        if (!failure) { assert(status == SOURCE_COPY_MAINS_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_COPY_MAINS_MEMORY && source_clone_map_seal(&map) != SOURCE_CLONE_OK); }
        failure_at = 0; source_clone_map_destroy(&map); assert(live_allocations == baseline);
    }
    source_copy_destinations_destroy(&result); source_clone_map_destroy(&explicit_map); finish_copy();
}
