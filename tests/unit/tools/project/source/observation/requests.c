/** @file
 * @brief Verify explicit deep opaque observations, absence proofs and independent fault retirement.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "../../../../../../tools/project/source/image/projection.h"
/** Author excluded-from-discovery opaque assets before admitting the original. */
static void prepare_requests(void) {
    setup_test(); selection_fixtures();
    directory("generated/project/packages/vendor/pkg/v1.0.0/src/raw.assets");
    directory("generated/project/packages/vendor/pkg/v1.0.0/src/raw.assets/deep");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/raw.assets/deep/data.bin", "opaque\r\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/raw.assets/unrequested.bin", "unrequested\n");
    selection_capture_scope(false, false);
}
/** Borrow stable absolute input keys from owner scratch, before measured build. */
static const char *request_key(SourceObservedImage *observed, const char *suffix) {
    const char *path = source_arena_join(&observed->arena, capture.roots[0].path, "/", suffix); assert(path); return path;
}
/** Requests traverse unknown opaque ancestors, preserve exact bytes, reuse
 * observations, and prove optional descendant absence from one complete list. */
static void it_observes_explicit_deep_inputs_and_replays_them_without_disk_fallback(void) {
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        prepare_requests(); SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        SourceObservationRequest requests[] = {
            {request_key(&observed, "packages/vendor/pkg/v1.0.0/src/raw.assets/deep/data.bin"), SOURCE_OBSERVE_CONTENT, false},
            {request_key(&observed, "packages/vendor/pkg/v1.0.0/src/raw.assets/deep"), SOURCE_OBSERVE_CHILDREN, false},
            {request_key(&observed, "packages/vendor/pkg/v1.0.0/src/raw.assets/missing/descendant"), SOURCE_OBSERVE_CONTENT, true},
            {NULL, SOURCE_OBSERVE_CONTENT, false}};
        requests[3] = requests[0];
        if (reverse) { SourceObservationRequest swap = requests[0]; requests[0] = requests[2]; requests[2] = swap; }
        assert(source_observed_image_build(&observed, &capture, requests, 4) == SOURCE_OBSERVATION_OK);
        const SourceCatalogImageEntry *data = observed_entry(&observed, "/raw.assets/deep/data.bin");
        assert(data && data->content && source_length(data->content) == 8 && !memcmp(source_bytes(data->content), "opaque\r\n", 8));
        const SourceCatalogImageEntry *opaque = observed_entry(&observed, "/raw.assets/unrequested.bin");
        assert(opaque && !opaque->metadata_captured && !opaque->content);
        const SourceCatalogImageEntry *ignored = observed_entry(&observed, "/dependencies/vendor/pkg/v1.0.0");
        assert(ignored && !ignored->metadata_captured && !observed_entry(&observed, "/missing/descendant"));
        ModuleCatalog replay; module_catalog_init(&replay, sources.allocator); const SourceText *text = NULL;
        fixture_io_begin(0);
        assert(module_catalog_build_view(&replay, observed.image.roots, observed.image.root_count, &observed.image.reader) == MODULE_OK);
        assert(observed.image.reader.read(&observed.image, data->path, &text) == SOURCE_OK && text == data->content);
        struct stat metadata;
        assert(observed.image.reader.tree.root(&observed.image, &replay,
            observed_entry(&observed, "/raw.assets/deep")->path, &metadata) == MODULE_OK);
        assert(fixture_io_end() == 0); compare_catalog(&capture.catalog, &replay);
        SourceImageChange addition = {SOURCE_IMAGE_ADD, *data};
        addition.entry.path = request_key(&observed, "packages/vendor/pkg/v1.0.0/src/raw.assets/deep/copied.bin");
        SourceCatalogImage projected = {0}; SourceProjectionIssue projection;
        assert(source_image_project(&observed.arena, &observed.image, &addition, 1, &projected, &projection) == SOURCE_PROJECTION_OK);
        text = NULL; fixture_io_begin(0);
        assert(projected.reader.read(&projected, addition.entry.path, &text) == SOURCE_OK && text == data->content);
        assert(fixture_io_end() == 0);
        module_catalog_destroy(&replay); source_observed_image_destroy(&observed); finish();
    }
}
/** Neither optionality nor opaque placement hides wrong types, required missing
 * inputs, path traversal, outside scope or invalid request kinds. */
static void it_refuses_invalid_or_unavailable_explicit_observation_demands(void) {
    const char *suffixes[] = {"packages/vendor/pkg/v1.0.0/src/raw.assets/missing/deep",
        "packages/vendor/pkg/v1.0.0/src/raw.assets", "packages/vendor/pkg/v1.0.0/src/raw.assets/deep/data.bin",
        "packages/../packages", "packages//vendor", "packages/", NULL, "packages", "packages/./vendor"};
    for (size_t i = 0; i < sizeof(suffixes) / sizeof(*suffixes); i++) {
        prepare_requests(); SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        const char *path = suffixes[i] ? request_key(&observed, suffixes[i]) : "/outside";
        SourceObservationRequest request = {path, i == 2 ? SOURCE_OBSERVE_CHILDREN : SOURCE_OBSERVE_CONTENT, i != 0};
        if (i == 7) { request.kind = (SourceObservationKind)99; }
        SourceObservationStatus expected = i < 3 ? SOURCE_OBSERVATION_PHYSICAL : SOURCE_OBSERVATION_INVALID;
        assert(source_observed_image_build(&observed, &capture, &request, 1) == expected);
        assert(!observed.valid && !observed.image.valid && observed.failed_path);
        source_observed_image_destroy(&observed); finish();
    }
}
/** Fail every allocation and physical directory/probe operation of the actual
 * demanded build; each failed prefix releases back to independent original owners. */
static void it_retires_every_requested_observation_allocation_and_physical_failure(void) {
    for (unsigned physical = 0; physical < 2; physical++) {
        prepare_requests(); size_t total = 0;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
            SourceObservationRequest request = {request_key(&observed, "packages/vendor/pkg/v1.0.0/src/raw.assets/deep/data.bin"), SOURCE_OBSERVE_CONTENT, false};
            size_t baseline = live_allocations, before = allocation_count;
            if (physical) { fixture_io_begin(failure); } else if (failure) { failure_at = before + failure; }
            SourceObservationStatus status = source_observed_image_build(&observed, &capture, &request, 1);
            size_t measured = physical ? fixture_io_end() : allocation_count - before;
            if (!failure) { assert(status == SOURCE_OBSERVATION_OK); total = measured; assert(total); }
            else { assert(status != SOURCE_OBSERVATION_OK && !observed.valid && !observed.image.valid); }
            failure_at = 0; source_observed_image_destroy(&observed);
            /* request_key owns one arena allocation before the measured build. */
            assert(live_allocations + 1 == baseline);
        }
        finish();
    }
}
