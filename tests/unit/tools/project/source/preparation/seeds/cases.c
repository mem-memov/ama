/** @file
 * @brief Check command seed composition from genuine standard, draft, ordinary and cyclic originals.
 * @see [T290](../../../../../../../docs/implementation/source-operations.md#t290-command-relation-construction-after-t292)
 */
/** Explicit raw placement fixes version storage, while shared closure owns real
 * cycles/tests. Draft mains still fix a designated version without active code. */
static void it_joins_actual_command_seeds_and_requires_explicit_cross_version_policy(void) {
    for (unsigned profile = 0; profile < 7; profile++) {
        seed_fixture_begin(profile);
        assert(project_source_preparation_seed(&seed_preparation)
            == (profile == 2 ? PROJECT_SOURCE_PREPARATION_SEEDS : PROJECT_SOURCE_PREPARATION_OK));
        ProjectSourceSeeds *seeds = &seed_preparation.seeds;
        assert(seeds->attempted && seeds->valid == (profile != 2));
        assert(seeds->status == (profile == 2 ? PROJECT_SOURCE_SEEDS_POLICY : PROJECT_SOURCE_SEEDS_OK));
        assert(!seed_preparation.complete.attempted && !seed_preparation.final.attempted);
        if (profile != 2) {
            assert(seeds->explicit_mains.ready && seeds->copy.valid && seeds->destinations.valid);
            assert(seeds->version_count == (size_t)(profile != 0 && profile != 5));
            if (seeds->version_count) {
                const char *version = profile < 2 ? "/vendor/family/pkg/v1.0.0" : "/vendor/family/pkg/v2.0.0";
                assert(!strcmp(seeds->versions[0].identity, version));
                const char *suffix = profile < 2 ? "/packages/vendor/family/pkg/v1.0.0" : "/packages/vendor/family/pkg/v2.0.0";
                size_t total = strlen(seeds->versions[0].physical), length = strlen(suffix);
                assert(total >= length && !strcmp(seeds->versions[0].physical + total - length, suffix));
            }
            if (profile == 4) {
                assert(seeds->destinations.count == 4);
                const char *expected[] = {"/vendor/family/pkg/v2.0.0/src/copied", "/vendor/family/pkg/v2.0.0/src/b",
                    "/vendor/family/pkg/v2.0.0/tests/copied/check", "/vendor/family/pkg/v2.0.0/tests/b/check"};
                for (size_t i = 0; i < sizeof(expected) / sizeof(*expected); i++) {
                    size_t found = 0;
                    for (size_t j = 0; j < seeds->destinations.count; j++) { found += !strcmp(expected[i], seeds->destinations.entries[j].identity); }
                    assert(found == 1);
                }
            }
            if (profile == 6) {
                /* Structural activation includes explicitly selected drafts;
                 * it is distinct from an executable Ama definition. */
                size_t count = 0;
                for (const SourceCloneMapping *row = seeds->explicit_mains.first; row; row = row->next) { count++; }
                assert(count == 4 && seeds->version_count == 1 && seeds->destinations.count == 4);
            }
            if (profile == 5) { assert(!seeds->explicit_mains.first && !seeds->destinations.count && !seeds->copy.seed_count); }
        }
        if (profile == 2) { assert(seeds->failed_row && seeds->failed_path == seeds->failed_row->path); }
        if (profile == 3) {
            SourceVersionPlacement untouched = {"unchanged identity", "unchanged physical"};
            SourceCloneMapping bad = *seeds->explicit_mains.first;
            bad.identity = "/vendor/family/pkg/v9.0.0/src/copied";
            assert(project_source_seeds_version(seeds, &bad, &untouched) == PROJECT_SOURCE_SEEDS_VERSION);
            assert(!strcmp(untouched.identity, "unchanged identity") && !strcmp(untouched.physical, "unchanged physical"));
            bad = *seeds->explicit_mains.first; bad.path = "/outside/prepared/roots";
            assert(project_source_seeds_version(seeds, &bad, &untouched) == PROJECT_SOURCE_SEEDS_VERSION);
            assert(!strcmp(untouched.identity, "unchanged identity") && !strcmp(untouched.physical, "unchanged physical"));
        }
        size_t before = allocation_count;
        ProjectSourceSeedsStatus saved = seeds->status;
        assert(project_source_preparation_seed(&seed_preparation) == PROJECT_SOURCE_PREPARATION_INVALID);
        assert(allocation_count == before && seeds->status == saved);
        seed_fixture_end(profile);
    }
}
