/** @file
 * @brief Compose version initialization with explicit existing/missing final roots and real catalog discovery.
 * @see [T276 checkpoint](../../../../../../../docs/implementation/source-operations.md#t276-remaining-implementation-steps-after-t283)
 */
#include "../../../../../../../tools/project/source/copy/configuration/version.h"
/** The actual captured source configuration supplies regular-file permissions. */
static const SourceCatalogImageEntry *domain_configuration_mode(const DomainFixture *fixture) {
    const ModuleVersion *version = fixture->original.units->source->version; assert(version);
    const SourceCatalogImageEntry *row = image_assembly_entry(&fixture->observed.image, source_path(version->configuration));
    assert(row && row->metadata_captured && S_ISREG(row->metadata.st_mode)); return row;
}
/** New package layout and the new semantic root enter one simultaneous image.
 * Original physical bytes remain present and no destination is written on disk. */
static void it_initializes_version_under_existing_or_missing_final_roots_with_one_image_projection(void) {
    for (unsigned mode = 1; mode <= 2; mode++) {
        DomainFixture fixture; domain_begin(&fixture, mode);
        SourceFileMap files; source_file_map_init(&files, sources.allocator);
        assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        SourceManager texts; source_manager_init(&texts, sources.allocator);
        SourceVersionPlacement version = {"/target/new/v3.0.0", domain_path(&fixture, fixture.destination, "packages/target/new/v3.0.0")};
        SourceVersionInitialization initialized; SourceVersionInitializationIssue issue;
        assert(source_version_initialize(&fixture.arena, &texts, &fixture.observed, &fixture.final_root, 1,
            &version, &files, domain_configuration_mode(&fixture), NULL, 0, &initialized, &issue) == SOURCE_VERSION_INITIALIZATION_OK);
        assert(initialized.final_roots == &fixture.final_root && initialized.final_root_count == 1 && initialized.count == 3);
        const SourceCatalogImageEntry *directory_mode = image_assembly_entry(&fixture.observed.image,
            mode == 1 ? fixture.destination : fixture.parent);
        assert(directory_mode && initialized.changes[1].entry.metadata.st_mode == directory_mode->metadata.st_mode
            && initialized.changes[2].entry.metadata.st_mode == directory_mode->metadata.st_mode);
        const SourceCatalogImage *image = NULL; SourceCopyImageIssue assembly;
        assert(source_copy_image_prepare(&fixture.arena, &fixture.observed.image, &files, NULL,
            initialized.changes, initialized.count, &fixture.final_root, 1, &image, &assembly) == SOURCE_COPY_IMAGE_OK);
        SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
        assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK && !final.view.count);
        SourcePackageEnvironment environment; source_package_environment_init(&environment, sources.allocator);
        assert(source_package_environment_build(&environment, &final.catalog) == SOURCE_ENVIRONMENT_OK);
        assert(environment.first && !environment.first->next && !strcmp(environment.first->version->logical_path, version.identity));
        assert(!source_length(environment.first->document.source) && !environment.first->selection->count && !environment.first->aliases->count);
        assert(access(version.physical, F_OK) != 0 && errno == ENOENT);
        source_package_environment_destroy(&environment); source_image_capture_destroy(&final);
        source_manager_destroy(&texts); source_file_map_destroy(&files); domain_end(&fixture);
    }
}
/** Missing/invalid final-root authority cannot fall back to original roots;
 * every new-root initialization allocation failure preserves output ownership. */
static void it_refuses_final_root_initializer_boundaries_and_retires_each_allocation_failure(void) {
    DomainFixture fixture; domain_begin(&fixture, 2);
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    SourceManager texts; source_manager_init(&texts, sources.allocator);
    const SourceCatalogImageEntry *mode = domain_configuration_mode(&fixture);
    SourceVersionPlacement version = {"/target/new/v3.0.0", domain_path(&fixture, fixture.destination, "packages/target/new/v3.0.0")};
    SourceVersionInitialization output = {.count = 99}; SourceVersionInitializationIssue issue;
    assert(source_version_initialize(&fixture.arena, &texts, &fixture.observed, NULL, 1,
        &version, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_INVALID);
    assert(source_version_initialize(&fixture.arena, &texts, &fixture.observed, NULL, 0,
        &version, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_PLACEMENT);
    assert(output.count == 99);
    ModuleRoot root = fixture.final_root; root.kind = MODULE_ROOT_STANDARD;
    assert(source_version_initialize(&fixture.arena, &texts, &fixture.observed, &root, 1,
        &version, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_PLACEMENT && output.count == 99);
    root = fixture.final_root; const char *excluded = "packages/target"; root.excluded = &excluded; root.excluded_count = 1;
    assert(source_version_initialize(&fixture.arena, &texts, &fixture.observed, &root, 1,
        &version, &files, mode, NULL, 0, &output, &issue) == SOURCE_VERSION_INITIALIZATION_PLACEMENT && output.count == 99);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceManager attempted; source_manager_init(&attempted, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceVersionInitializationStatus status = source_version_initialize(&arena, &attempted, &fixture.observed,
            &fixture.final_root, 1, &version, &files, mode, NULL, 0, &output, &issue);
        if (!failure) { assert(status == SOURCE_VERSION_INITIALIZATION_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_VERSION_INITIALIZATION_MEMORY && output.count == 99); }
        failure_at = 0; source_manager_destroy(&attempted); source_arena_destroy(&arena); assert(live_allocations == baseline);
        output = (SourceVersionInitialization){.count = 99};
    }
    source_manager_destroy(&texts); source_file_map_destroy(&files); domain_end(&fixture);
}
