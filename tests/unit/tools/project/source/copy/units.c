/** @file
 * @brief Compose opaque real unit captures with mapped causes, physical conflicts and allocation failures.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Capture raw companions/plus data before the immutable original is admitted. */
static void prepare_unit_members(void) {
    setup_test(); fixtures();
    file("generated/project/packages/vendor/pkg/v1.0.0/src/main.payload", "opaque companion\n");
    directory("generated/project/packages/vendor/pkg/v1.0.0/src/main+");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/main+/data.bin", "opaque subtree data\n");
    capture_fixtures(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
}
/** Retire all borrowers before removing this profile's extra opaque fixtures. */
static void finish_unit_members(void) {
    finish_copy();
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/main.payload") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/main+/data.bin") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/src/main+") == 0);
}
/** Build actual ordinary destinations and a complete active main relation. */
static void unit_main_map(SourceCloneMap *explicit_map, SourceCopyDestinations *destinations, SourceCloneMap *mains) {
    main_destination(explicit_map); source_copy_destinations_init(destinations, sources.allocator);
    assert(source_copy_destinations_build(destinations, &copy, explicit_map) == SOURCE_COPY_DESTINATIONS_OK && destinations->count == 4);
    const char *names[4];
    for (size_t i = 0; i < 4; i++) { names[i] = destinations->entries[i].identity; }
    source_clone_map_init(mains, sources.allocator, &capture);
    SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"}; SourceCopyMainsIssue issue;
    assert(source_copy_place_version_mains(mains, destinations, &version, names, 4, &issue) == SOURCE_COPY_MAINS_OK);
    assert(source_clone_map_seal(mains) == SOURCE_CLONE_OK);
}
/** All original unit members are mapped byte-exactly; test-directory/configuration
 * causes remain separate passes, and equal repeat causes agree instead of renaming. */
static void it_maps_real_opaque_unit_companions_and_plus_data_with_durable_capture_descriptors(void) {
    prepare_unit_members(); SourceCloneMap explicit_map, mains; SourceCopyDestinations destinations;
    unit_main_map(&explicit_map, &destinations, &mains);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue issue;
    assert(source_copy_map_unit_members(&files, &copy, &mains, NULL, &issue) == SOURCE_COPY_UNITS_OK && !files.ready);
    size_t count = 0; bool companion = false, data = false, plus = false;
    for (const SourceFileMapping *row = files.first; row; row = row->next) {
        count++;
        if (!strcmp(row->destination, "/prepared/src/renamed.payload")) {
            companion = true; assert(!row->original->directory && source_length(row->original->file.content) == 17);
            assert(!memcmp(source_bytes(row->original->file.content), "opaque companion\n", 17));
        }
        if (!strcmp(row->destination, "/prepared/src/renamed+/data.bin")) { data = true; assert(!row->original->directory); }
        if (!strcmp(row->destination, "/prepared/src/renamed+")) { plus = true; assert(row->original->directory); }
        assert(!strstr(row->destination, "package.ini") && !strstr(row->destination, "README.md"));
    }
    assert(count == 7 && companion && data && plus);
    assert(source_copy_map_unit_members(&files, &copy, &mains, NULL, &issue) == SOURCE_COPY_UNITS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    assert(source_copy_map_unit_members(&files, &copy, &mains, NULL, &issue) == SOURCE_COPY_UNITS_INVALID && files.ready);
    source_file_map_destroy(&files); source_clone_map_destroy(&mains); source_copy_destinations_destroy(&destinations);
    source_clone_map_destroy(&explicit_map); finish_unit_members();
}
/** Missing main correspondence and existing conflicting physical proposals refuse
 * before a partial result can be sealed; all measured allocations retire. */
static void it_refuses_unmapped_or_conflicting_unit_causes_and_retires_every_allocation_failure(void) {
    prepare_unit_members(); SourceCloneMap explicit_map, mains; SourceCopyDestinations destinations;
    unit_main_map(&explicit_map, &destinations, &mains);
    SourceCloneMap empty; source_clone_map_init(&empty, sources.allocator, &capture);
    assert(source_clone_map_seal(&empty) == SOURCE_CLONE_OK);
    SourceFileMap missing; source_file_map_init(&missing, sources.allocator); SourceCopyUnitsIssue issue;
    assert(source_copy_map_unit_members(&missing, &copy, &empty, NULL, &issue) == SOURCE_COPY_UNITS_MAIN && issue.member && issue.cause && !issue.clone);
    assert(source_file_map_seal(&missing) != SOURCE_FILE_MAP_OK); source_file_map_destroy(&missing); source_clone_map_destroy(&empty);
    SourceFileMap conflict; source_file_map_init(&conflict, sources.allocator);
    const AcquisitionMember *member = copy.closure.members.first;
    while (member && !strstr(member->path, "/src/main.payload")) { member = member->next; } assert(member);
    const SourceFileMapping *row = NULL;
    assert(source_file_map_observation(&conflict, member->path, member->content, false, "/wrong", &row) == SOURCE_FILE_MAP_OK);
    assert(source_copy_map_unit_members(&conflict, &copy, &mains, NULL, &issue) == SOURCE_COPY_UNITS_MAP);
    assert(conflict.status == SOURCE_FILE_MAP_CONFLICT && conflict.requester && !strcmp(conflict.requester->file.path, member->path));
    assert(source_file_map_seal(&conflict) != SOURCE_FILE_MAP_OK); source_file_map_destroy(&conflict);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceFileMap files; source_file_map_init(&files, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyUnitsStatus status = source_copy_map_unit_members(&files, &copy, &mains, NULL, &issue);
        if (!failure) { assert(status == SOURCE_COPY_UNITS_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_COPY_UNITS_MEMORY && source_file_map_seal(&files) != SOURCE_FILE_MAP_OK); }
        failure_at = 0; source_file_map_destroy(&files); assert(live_allocations == baseline);
    }
    SourceFileMap invalid; source_file_map_init(&invalid, sources.allocator);
    assert(source_copy_map_unit_members(NULL, &copy, &mains, NULL, &issue) == SOURCE_COPY_UNITS_INVALID);
    assert(source_copy_map_unit_members(&invalid, &copy, &mains, NULL, NULL) == SOURCE_COPY_UNITS_INVALID && invalid.status == SOURCE_FILE_MAP_OK);
    assert(source_copy_map_unit_members(&invalid, NULL, &mains, NULL, &issue) == SOURCE_COPY_UNITS_INVALID && invalid.status == SOURCE_FILE_MAP_INVALID);
    source_file_map_destroy(&invalid); source_clone_map_destroy(&mains); source_copy_destinations_destroy(&destinations);
    source_clone_map_destroy(&explicit_map); finish_unit_members();
}
