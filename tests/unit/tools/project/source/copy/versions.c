/** @file
 * @brief Verify absent-version structural changes, carried files and initialization ownership.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Select the actual original configuration mode row, never a fabricated stat prototype. */
static const SourceCatalogImageEntry *version_mode(const SourceObservedImage *observed) {
    const ModuleSource *main = NULL; assert(module_catalog_find(&capture.catalog, main_identity, &main) == MODULE_OK);
    const char *path = source_path(main->version->configuration);
    for (size_t i = 0; i < observed->image.count; i++) { if (!strcmp(observed->image.entries[i].path, path)) { return &observed->image.entries[i]; } }
    assert(false); return NULL;
}
/** Empty versions retain both required areas; previously reserved symbolic
 * classes remain distinct and final common discovery gives empty strict scopes. */
static void it_initializes_required_empty_version_areas_and_preserves_reserved_identity_classes(void) {
    prepare(); SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourceManager texts; source_manager_init(&texts, sources.allocator);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    SourceVersionInitialization versions[2]; SourceVersionInitializationIssue issue;
    const SourceCatalogImageEntry *mode = version_mode(&observed);
    const char *physical = source_arena_join(&arena, capture.roots[0].path, "/", "packages/target/new/v3.0.0"); assert(physical);
    SourceVersionPlacement placement = {"/target/new/v3.0.0", physical};
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, mode, NULL, 0, &versions[0], &issue) == SOURCE_VERSION_INITIALIZATION_OK);
    assert(versions[0].count == 3 && source_length(versions[0].configuration) == 0);
    SourceVersionInitialization sentinel = {.count = 17};
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, mode,
        versions[0].changes, 3, &sentinel, &issue) == SOURCE_VERSION_INITIALIZATION_CONFLICT);
    assert(sentinel.count == 17);
    placement = (SourceVersionPlacement){"/target/new/v4.0.0", source_arena_join(&arena, capture.roots[0].path, "/", "packages/target/new/v4.0.0")};
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, mode, versions[0].changes, 3, &versions[1], &issue) == SOURCE_VERSION_INITIALIZATION_OK);
    SourceImageChange changes[6];
    for (size_t i = 0; i < 6; i++) {
        changes[i] = versions[i / 3].changes[i % 3];
        for (size_t j = 0; j < i; j++) { assert(changes[i].entry.metadata.st_dev != changes[j].entry.metadata.st_dev
            || changes[i].entry.metadata.st_ino != changes[j].entry.metadata.st_ino); }
    }
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&arena, &observed.image, &files, NULL, changes, 6, observed.image.roots, observed.image.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    SourcePackageEnvironment environment; source_package_environment_init(&environment, sources.allocator);
    assert(source_package_environment_build(&environment, &final.catalog) == SOURCE_ENVIRONMENT_OK);
    for (size_t i = 0; i < 2; i++) {
        const SourcePackageScope *scope = environment_scope(&environment, versions[i].version.identity);
        assert(!source_length(scope->document.source) && !scope->aliases->count && !scope->selection->count);
        const char *area = source_arena_join(&arena, versions[i].version.physical, "/", "src");
        assert(image_assembly_entry(image, area) && S_ISDIR(image_assembly_entry(image, area)->metadata.st_mode));
        area = source_arena_join(&arena, versions[i].version.physical, "/", "tests");
        assert(image_assembly_entry(image, area) && S_ISDIR(image_assembly_entry(image, area)->metadata.st_mode));
    }
    source_package_environment_destroy(&environment); source_image_capture_destroy(&final);
    source_file_map_destroy(&files); source_manager_destroy(&texts); source_arena_destroy(&arena); source_observed_image_destroy(&observed); finish();
}
/** Explicit carried package files and bad placements never become new empty
 * scopes; every allocation failure preserves output and releases both owners. */
static void it_refuses_carried_existing_and_foreign_version_initialization_and_retires_failures(void) {
    prepare(); SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourceManager texts; source_manager_init(&texts, sources.allocator);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    const SourceCatalogImageEntry *mode = version_mode(&observed);
    SourceVersionPlacement placement = {"/target/new/v3.0.0", source_arena_join(&arena, capture.roots[0].path, "/", "packages/target/new/v3.0.0")};
    SourceVersionInitialization sentinel = {.original = &observed, .count = 17}, output = sentinel; SourceVersionInitializationIssue issue;
    SourceCatalogImageEntry foreign = *mode;
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, &foreign, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_INVALID);
    SourceVersionPlacement wrong = {"/wrong/name/v3.0.0", placement.physical};
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &wrong, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_PLACEMENT);
    SourceVersionPlacement old = {"/vendor/pkg/v1.0.0", source_arena_join(&arena, capture.roots[0].path, "/", "packages/vendor/pkg/v1.0.0")};
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &old, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_ABSENCE);
    assert(output.original == sentinel.original && output.count == sentinel.count);
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena attempt; source_arena_init(&attempt, sources.allocator); SourceManager attempted_texts; source_manager_init(&attempted_texts, sources.allocator);
        size_t start = allocation_count; if (failure) { failure_at = start + failure; } output = sentinel;
        SourceVersionInitializationStatus status = source_version_initialize(&attempt, &attempted_texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, mode, NULL, 0, &output, &issue);
        if (!failure) { assert(status == SOURCE_VERSION_INITIALIZATION_OK); total = allocation_count - start; assert(total); }
        else { assert(status == SOURCE_VERSION_INITIALIZATION_MEMORY && output.original == sentinel.original && output.count == sentinel.count); }
        failure_at = 0; source_manager_destroy(&attempted_texts); source_arena_destroy(&attempt); assert(live_allocations == baseline);
    }
    source_file_map_destroy(&files); source_file_map_init(&files, sources.allocator); const SourceFileMapping *row = NULL;
    const char *destination = source_arena_join(&arena, placement.physical, "/", "package.ini");
    assert(source_file_map_observation(&files, mode->path, mode->content, false, destination, &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    assert(source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_CARRIED);
    assert(output.count == sentinel.count);
    source_file_map_destroy(&files);
    for (unsigned area = 0; area < 3; area++) {
        source_file_map_init(&files, sources.allocator);
        const char *suffix = area == 0 ? "src" : area == 1 ? "src/item" : "tests/item";
        const char *target = source_arena_join(&arena, placement.physical, "/", suffix);
        assert(source_file_map_observation(&files, mode->path, mode->content, false, target, &row) == SOURCE_FILE_MAP_OK);
        assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        SourceVersionInitializationStatus status = source_version_initialize(&arena, &texts, &observed, observed.image.roots, observed.image.root_count, &placement, &files, mode, NULL, 0, &output, &issue);
        if (!area) { assert(status == SOURCE_VERSION_INITIALIZATION_CONFLICT); }
        else {
            assert(status == SOURCE_VERSION_INITIALIZATION_OK && output.count == 2);
            const char *required = area == 1 ? "/tests" : "/src";
            assert(!strcmp(output.changes[1].entry.path + strlen(placement.physical), required));
        }
        source_file_map_destroy(&files);
    }
    source_manager_destroy(&texts); source_arena_destroy(&arena); source_observed_image_destroy(&observed); finish();
}
