/** @file
 * @brief Compose actual copy closures with explicit seeds, additional names and test destinations.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/destinations/destinations.h"
#include "../../../../../../tools/project/source/mapping/physical/main.h"
#include "../../../../../../tools/project/source/copy/destinations/physical.h"
/** Find a logical output by its exact original catalog identity. */
static const SourceCopyDestination *destination(const SourceCopyDestinations *result, const char *original) {
    for (size_t i = 0; i < result->count; i++) {
        if (!strcmp(result->entries[i].source->logical_path, original)) { return &result->entries[i]; }
    }
    return NULL;
}
/** Use an actual selected original handle and one explicitly chosen candidate.
 * Final physical/catalog approval remains outside this logical-phase fixture. */
static void main_destination(SourceCloneMap *explicit_map) {
    source_clone_map_init(explicit_map, sources.allocator, &capture);
    const SourceCloneMapping *row = NULL;
    assert(tree.count == 1);
    assert(source_clone_map_add(explicit_map, tree.units[0]->source, "/prepared/src/renamed",
        "/target/new/v3.0.0/src/renamed", &row) == SOURCE_CLONE_OK);
}
/** Actual cyclic closure joins mode membership, version propagation, source
 * suffixes, associated tests, final plus construction and injective proposals. */
static void it_composes_actual_copy_closures_with_logical_and_physical_destination_candidates(void) {
    const SourceCopyDependencyMode modes[] = {SOURCE_COPY_KEEP_EXTERNAL, SOURCE_COPY_EXTEND_PACKAGE, SOURCE_COPY_CLONE_DEPENDENCIES};
    for (size_t mode = 0; mode < 3; mode++) {
        prepare_profile(true, false, false); select_main();
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, modes[mode]) == SOURCE_COPY_CLOSURE_OK);
        SourceCloneMap explicit_map; main_destination(&explicit_map);
        SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
        assert(source_copy_destinations_build(&result, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK && result.valid);
        assert(result.count == (mode == 2 ? 4u : 2u));
        assert(!strcmp(destination(&result, main_identity)->identity, "/target/new/v3.0.0/src/renamed"));
        assert(!strcmp(destination(&result, "/vendor/pkg/v1.0.0/tests/main/check")->identity,
            "/target/new/v3.0.0/tests/renamed/check"));
        if (mode == 2) {
            assert(!strcmp(destination(&result, "/vendor/pkg/v1.0.0/src/other")->identity, "/target/new/v3.0.0/src/other"));
            assert(!strcmp(destination(&result, "/vendor/pkg/v1.0.0/tests/other/check")->identity,
                "/target/new/v3.0.0/tests/other/check"));
        }
        SourceCloneMap complete; source_clone_map_init(&complete, sources.allocator, &capture);
        const char *names[4];
        for (size_t i = 0; i < result.count; i++) { names[i] = result.entries[i].identity; }
        SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"};
        SourceCopyMainsIssue placement;
        assert(source_copy_place_version_mains(&complete, &result, &version, names, result.count, &placement) == SOURCE_COPY_MAINS_OK);
        for (size_t i = 0; i < result.count; i++) {
            const SourceCloneMapping *row = complete.first;
            while (row && row->original != result.entries[i].source) { row = row->next; }
            assert(row && !strcmp(row->identity, result.entries[i].identity));
        }
        assert(source_clone_map_seal(&complete) == SOURCE_CLONE_OK);
        size_t before = allocation_count;
        assert(source_copy_destinations_build(&result, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_INVALID && result.valid);
        assert(allocation_count == before);
        source_clone_map_destroy(&complete); source_copy_destinations_destroy(&result);
        source_clone_map_destroy(&explicit_map); finish_copy();
    }
}
/** Every destination-owner allocation failure retains no published output and
 * releases all scratch without rerunning providers or independent test probes. */
static void it_retires_every_actual_copy_destination_allocation_failure(void) {
    prepare(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceCloneMap explicit_map; main_destination(&explicit_map);
    const SourceAnalysisProbe *probes = analysis.probes;
    size_t baseline = live_allocations, allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyDestinationsStatus status = source_copy_destinations_build(&result, &copy, &explicit_map);
        if (!failure) { assert(status == SOURCE_COPY_DESTINATIONS_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_COPY_DESTINATIONS_MEMORY && !result.valid && !result.entries); }
        failure_at = 0; source_copy_destinations_destroy(&result);
        assert(live_allocations == baseline && analysis.probes == probes);
    }
    source_clone_map_destroy(&explicit_map); finish_copy();
}
/** Explicit coverage, original capture identity and ordinary destination root
 * are all checked; no malformed candidate can silently borrow another source. */
static void it_refuses_invalid_copy_destination_inputs_and_namespace_crossings(void) {
    prepare(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    for (unsigned profile = 0; profile < 5; profile++) {
        SourceCloneMap explicit_map; source_clone_map_init(&explicit_map, sources.allocator, &capture);
        const SourceCloneMapping *row = NULL;
        if (profile) {
            assert(source_clone_map_add(&explicit_map, tree.units[0]->source, "/prepared/src/renamed",
                profile == 2 ? "//renamed" : "/target/new/v3.0.0/src/renamed", &row) == SOURCE_CLONE_OK);
        }
        if (profile == 3) {
            const ModuleSource *other = NULL;
            assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/other", &other) == MODULE_OK);
            assert(source_clone_map_add(&explicit_map, other, "/prepared/src/other", "/target/new/v3.0.0/src/other", &row) == SOURCE_CLONE_OK);
        }
        if (profile == 4) { explicit_map.capture = NULL; }
        SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
        SourceCopyDestinationsStatus status = source_copy_destinations_build(&result, &copy, &explicit_map);
        if (profile == 1) { assert(status == SOURCE_COPY_DESTINATIONS_OK && result.valid); }
        else { assert(status == (profile == 2 ? SOURCE_COPY_DESTINATIONS_NAMESPACE : SOURCE_COPY_DESTINATIONS_INVALID)); assert(!result.valid && !result.entries); }
        source_copy_destinations_destroy(&result); source_clone_map_destroy(&explicit_map);
    }
    SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
    assert(source_copy_destinations_build(NULL, &copy, NULL) == SOURCE_COPY_DESTINATIONS_INVALID);
    assert(source_copy_destinations_build(&result, NULL, NULL) == SOURCE_COPY_DESTINATIONS_INVALID && !result.valid);
    source_copy_destinations_destroy(&result); finish_copy();
}

/** Candidate-stage fixtures deliberately assign nested source originals to
 * different versions. Only logical projection is certified here; explicit raw
 * physical P agreement remains a separate mandatory outer check. */
static void it_selects_deepest_original_test_owners_and_refuses_shared_helper_version_conflicts(void) {
    for (unsigned shared = 0; shared < 2; shared++) {
        setup_test(); fixtures();
        file("generated/project/packages/vendor/pkg/v1.0.0/src/parent", "@!\n");
        file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/check", "@!\n");
        if (shared) {
            file("generated/project/packages/vendor/pkg/v1.0.0/tests/support", "@!\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/tests/main/check", "@!\nshared&\n../support\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/tests/other/check", "@!\nshared&\n../support\n");
        }
        capture_fixtures(); source_tree_selection_init(&tree, sources.allocator);
        assert(source_tree_selection_read(&tree, &capture, "generated/project/packages/vendor/pkg/v1.0.0/src") == SOURCE_TREE_OK);
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
        SourceCloneMap explicit_map; source_clone_map_init(&explicit_map, sources.allocator, &capture);
        for (size_t i = 0; i < tree.count; i++) {
            const ModuleSource *source = tree.units[i]->source;
            const char *identity = NULL, *path = NULL;
            if (!strcmp(source->logical_path, main_identity)) { identity = "/target/a/v2.0.0/src/main"; path = "/prepared/main"; }
            else if (!strcmp(source->logical_path, "/vendor/pkg/v1.0.0/src/other")) { identity = "/target/b/v2.0.0/src/other"; path = "/prepared/other"; }
            else if (!strcmp(source->logical_path, child_identity)) { identity = "/target/b/v2.0.0/src/inner"; path = "/prepared/inner"; }
            else { assert(!strcmp(source->logical_path, "/vendor/pkg/v1.0.0/src/parent")); identity = "/target/a/v2.0.0/src/outer"; path = "/prepared/outer"; }
            const SourceCloneMapping *row = NULL;
            assert(source_clone_map_add(&explicit_map, source, path, identity, &row) == SOURCE_CLONE_OK);
        }
        SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
        SourceCopyDestinationsStatus status = source_copy_destinations_build(&result, &copy, &explicit_map);
        if (shared) {
            assert(status == SOURCE_COPY_DESTINATIONS_VERSIONS && !result.valid && !result.entries);
            assert(result.version_status == SOURCE_VERSION_CONFLICT && result.failed_role);
            assert(!strcmp(result.failed_role->identity, "/vendor/pkg/v1.0.0/tests/support"));
            assert(strcmp(result.version_issue.first, result.version_issue.second));
        } else {
            assert(status == SOURCE_COPY_DESTINATIONS_OK && result.valid);
            assert(!strcmp(destination(&result, "/vendor/pkg/v1.0.0/tests/parent/check")->identity,
                "/target/a/v2.0.0/tests/outer/check"));
            assert(!strcmp(destination(&result, "/vendor/pkg/v1.0.0/tests/parent/child/check")->identity,
                "/target/b/v2.0.0/tests/inner/check"));
        }
        source_copy_destinations_destroy(&result); source_clone_map_destroy(&explicit_map); finish_copy();
    }
}
/** Standard-only and empty raw selections produce no invented ordinary version
 * or associated-test candidates. Draft src mains remain active structural units
 * but acquire no automatic test association of their own. */
static void it_handles_standard_empty_and_draft_destination_partitions(void) {
    const char *operands[] = {"generated/standard/plain", "generated/project/packages/vendor/pkg/v1.0.0/src/empty",
        "generated/project/packages/vendor/pkg/v1.0.0/src/parent"};
    for (unsigned profile = 0; profile < 3; profile++) {
        prepare();
        if (profile == 1) { directory(operands[1]); }
        source_tree_selection_init(&tree, sources.allocator);
        assert(source_tree_selection_read(&tree, &capture, operands[profile]) == SOURCE_TREE_OK);
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
        SourceCloneMap explicit_map; source_clone_map_init(&explicit_map, sources.allocator, &capture);
        for (size_t i = 0; i < tree.count; i++) {
            const char *identity = profile == 0 ? "//renamed" :
                !strcmp(tree.units[i]->source->logical_path, child_identity) ? "/target/a/v2.0.0/src/copy/child" : "/target/a/v2.0.0/src/copy";
            const char *path = profile == 0 ? "/prepared/renamed" :
                !strcmp(tree.units[i]->source->logical_path, child_identity) ? "/prepared/copy+/child" : "/prepared/copy";
            const SourceCloneMapping *row = NULL;
            assert(source_clone_map_add(&explicit_map, tree.units[i]->source, path, identity, &row) == SOURCE_CLONE_OK);
        }
        SourceCopyDestinations result; source_copy_destinations_init(&result, sources.allocator);
        assert(source_copy_destinations_build(&result, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK && result.valid);
        if (profile < 2) { assert(!result.count && !result.entries); }
        else {
            assert(result.count == 3 && !destination(&result, "/vendor/pkg/v1.0.0/tests/parent/check"));
            assert(!strcmp(destination(&result, "/vendor/pkg/v1.0.0/tests/parent/child/check")->identity,
                "/target/a/v2.0.0/tests/copy/child/check"));
        }
        source_copy_destinations_destroy(&result); source_clone_map_destroy(&explicit_map); finish_copy();
    }
}
