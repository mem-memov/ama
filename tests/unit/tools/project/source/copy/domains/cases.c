/** @file
 * @brief Compose actual compiler selection, independent retirement, forests and leased freshness.
 * @see [T275 contract](../../../../../../../docs/implementation/source-operations.md#t275-entry-review-and-execution)
 */
/** Promotion and root relocation keep nominal names, preserve every old file,
 * mutate only the requested destination, and retain exact opaque observations. */
static void it_proves_real_promotion_and_root_relocation_without_physical_source_removal(void) {
    for (unsigned relocate = 0; relocate < 3; relocate++) {
        DomainFixture fixture; domain_begin(&fixture, relocate); domain_project(&fixture);
        domain_prove(&fixture);
        SourceRegistryRetirement retired;
        assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, &fixture.final, NULL, 0, &retired) == SOURCE_REGISTRY_OK && retired.count == 4);
        SourceSitesIssue sites;
        assert(source_graph_sites(fixture.analysis.original.response.definitions, fixture.after.response.definitions, &fixture.map,
            NULL, 0, retired, fixture.prepared, &sites) == SOURCE_SITES_OK);
        SourceMutationForest forest; SourceForestIssue issue;
        assert(source_mutation_forest(&fixture.arena, &fixture.observed.image, &fixture.image, &forest, &issue) == SOURCE_FOREST_OK && forest.count == 1);
        assert(!strcmp(forest.roots[0].path, fixture.destination));
        assert((forest.roots[0].before != NULL) == (relocate == 1));
        for (const SourceCapturedUnit *old = fixture.original.units; old; old = old->next) {
            const char *path = source_path(old->source->text); bool found = false;
            for (size_t i = 0; i < fixture.image.count; i++) { found |= !strcmp(path, fixture.image.entries[i].path); }
            assert(found && access(path, F_OK) == 0);
        }
        SourceTransactionGuard guard; source_transaction_guard_init(&guard, sources.allocator);
        assert(source_transaction_guard_acquire(&guard, &fixture.observed, (SourceLeaseControl){0}) == SOURCE_GUARD_OK);
        assert(source_transaction_guard_destroy(&guard) == 0);
        file(fixture.opaque, "changed"); source_transaction_guard_init(&guard, sources.allocator);
        assert(source_transaction_guard_acquire(&guard, &fixture.observed, (SourceLeaseControl){0}) == SOURCE_GUARD_STALE);
        assert(guard.freshness_status == SOURCE_FRESH_CONTENT && !strcmp(guard.issue.path, fixture.opaque));
        assert(source_transaction_guard_destroy(&guard) == 0); domain_end(&fixture);
    }
}
/** Selection retirement is exact, output-atomic and never invents removal authority. */
static void it_retires_registry_scratch_failures_and_refuses_foreign_removal_names(void) {
    DomainFixture fixture; domain_begin(&fixture, false); domain_project(&fixture);
    SourceRegistryRetirement output = {.count = 99}; size_t live = live_allocations;
    failure_at = allocation_count + 1;
    assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, &fixture.final, NULL, 0, &output) == SOURCE_REGISTRY_MEMORY && output.count == 99);
    failure_at = 0; assert(live_allocations == live);
    const char *invalid[] = {NULL, "/unknown"};
    for (size_t i = 0; i < 2; i++) {
        assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, &fixture.final, &invalid[i], 1, &output) == SOURCE_REGISTRY_INVALID && output.count == 99);
    }
    assert(source_copy_registry_retirement(NULL, &fixture.original, &fixture.final, NULL, 0, &output) == SOURCE_REGISTRY_INVALID);
    assert(source_copy_registry_retirement(&fixture.arena, NULL, &fixture.final, NULL, 0, &output) == SOURCE_REGISTRY_INVALID);
    assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, NULL, NULL, 0, &output) == SOURCE_REGISTRY_INVALID);
    assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, &fixture.final, NULL, 1, &output) == SOURCE_REGISTRY_INVALID);
    assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, &fixture.final, NULL, 0, NULL) == SOURCE_REGISTRY_INVALID);
    assert(output.count == 99);
    SourceImageCapture unchanged; source_image_capture_init(&unchanged, sources.allocator);
    assert(source_image_capture_build(&unchanged, &fixture.observed.image) == SOURCE_IMAGE_CAPTURE_OK);
    size_t before = allocation_count; failure_at = before + 1;
    assert(source_copy_registry_retirement(&fixture.arena, &fixture.original, &unchanged, NULL, 0, &output) == SOURCE_REGISTRY_OK && !output.count);
    assert(allocation_count == before); failure_at = 0;
    source_image_capture_destroy(&unchanged); domain_end(&fixture);
}
/** Newly covered roots belong to original analysis, so a same-priority package
 * collision refuses before any clone/rename or final-image proof can hide it. */
static void it_rejects_original_conflicts_in_augmented_root_analysis(void) {
    DomainFixture fixture; domain_begin(&fixture, true);
    const char *directories[] = {"dependencies", "dependencies/vendor", "dependencies/vendor/pkg",
        "dependencies/vendor/pkg/v1.0.0", "dependencies/vendor/pkg/v1.0.0/src", "dependencies/vendor/pkg/v1.0.0/tests"};
    for (size_t i = 0; i < sizeof(directories)/sizeof(*directories); i++) { directory(domain_path(&fixture, fixture.destination, directories[i])); }
    file(domain_path(&fixture, fixture.destination, "dependencies/vendor/pkg/v1.0.0/package.ini"), "");
    file(domain_path(&fixture, fixture.destination, "dependencies/vendor/pkg/v1.0.0/src/main"), "@!\n");
    SourceRootCapture augmented; source_root_capture_init(&augmented, sources.allocator);
    assert(source_root_capture_build(&augmented, fixture.original.roots, fixture.original.root_count) == SOURCE_CAPTURE_CATALOG);
    assert(!augmented.valid && augmented.catalog.issue.status == MODULE_AMBIGUOUS && augmented.catalog.issue.related_path);
    source_root_capture_destroy(&augmented);
    assert(unlink(domain_path(&fixture, fixture.destination, "dependencies/vendor/pkg/v1.0.0/src/main")) == 0);
    assert(unlink(domain_path(&fixture, fixture.destination, "dependencies/vendor/pkg/v1.0.0/package.ini")) == 0);
    for (size_t i = sizeof(directories)/sizeof(*directories); i; i--) { assert(rmdir(domain_path(&fixture, fixture.destination, directories[i-1])) == 0); }
    domain_end(&fixture);
}
/** Wider parent traversal must retire every measured failed observation prefix. */
static void it_retires_each_wider_envelope_observation_allocation_failure(void) {
    DomainFixture fixture; domain_begin(&fixture, false); size_t total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        SourceObservationRequest request = {fixture.opaque, SOURCE_OBSERVE_CONTENT, false};
        size_t before = allocation_count, live = live_allocations;
        if (failure) { failure_at = before + failure; }
        SourceObservationStatus status = source_observed_image_build(&observed, &fixture.original,
            (SourceImageBounds){&fixture.parent, 1}, &request, 1);
        if (!failure) { assert(status == SOURCE_OBSERVATION_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_OBSERVATION_OK && !observed.valid && !observed.image.valid); }
        failure_at = 0; source_observed_image_destroy(&observed); assert(live_allocations == live);
    }
    domain_end(&fixture);
}

/** Copied tables retain keys/comments while their owned alias targets are rooted;
 * the resulting actual image still passes independent final compiler/site proof. */
static void it_owns_carried_package_configuration_aliases_without_importing_unselected_tables(void) {
    DomainFixture fixture;
    domain_begin_configuration(&fixture, 0, "; original spelling\n[aliases]\n~helper = ./src/helper\n");
    domain_project(&fixture); domain_prove(&fixture); domain_end(&fixture);
}
