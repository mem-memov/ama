/** @file
 * @brief Retire every measured command-seed allocation failure against immutable real originals.
 * @see [T290](../../../../../../../docs/implementation/source-operations.md#t290-command-relation-construction-after-t292)
 */
/** No test probe cache affects this draft/standard allocation enumeration.
 * Each failed sole build stays invalid, preserves primary evidence on repeat,
 * and releases all custom allocations without touching source or destination. */
static void it_retires_every_command_seed_allocation_failure_and_rejects_invalid_admission(void) {
    const unsigned profiles[] = {0, 3, 5};
    for (size_t index = 0; index < sizeof(profiles) / sizeof(*profiles); index++) {
        unsigned profile = profiles[index]; seed_fixture_begin(profile);
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            ProjectSourceSeeds seeds; project_source_seeds_init(&seeds, sources.allocator);
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            ProjectSourceSeedsStatus status = project_source_seeds_build(&seeds, &seed_preparation.placement,
                &seed_preparation.original, NULL, seed_request.dependencies,
                seed_preparation.root_settings.final, seed_preparation.root_settings.final_count);
            if (!failure) { assert(status == PROJECT_SOURCE_SEEDS_OK && seeds.valid); total = allocation_count - before; assert((total == 0) == (profile == 5)); }
            else { assert(status != PROJECT_SOURCE_SEEDS_OK && !seeds.valid); }
            failure_at = 0; before = allocation_count;
            assert(project_source_seeds_build(&seeds, NULL, NULL, NULL, SOURCE_COPY_KEEP_EXTERNAL, NULL, 0) == PROJECT_SOURCE_SEEDS_INVALID);
            assert(allocation_count == before && seeds.status == status);
            project_source_seeds_destroy(&seeds); assert(live_allocations == baseline);
        }
        ProjectSourceSeeds seeds; project_source_seeds_init(&seeds, sources.allocator);
        assert(project_source_seeds_build(&seeds, NULL, &seed_preparation.original, NULL,
            SOURCE_COPY_KEEP_EXTERNAL, NULL, 0) == PROJECT_SOURCE_SEEDS_INVALID && !seeds.valid);
        project_source_seeds_destroy(&seeds);
        assert(project_source_seeds_build(NULL, NULL, NULL, NULL, SOURCE_COPY_KEEP_EXTERNAL, NULL, 0) == PROJECT_SOURCE_SEEDS_INVALID);
        seed_fixture_end(profile);
    }
}
