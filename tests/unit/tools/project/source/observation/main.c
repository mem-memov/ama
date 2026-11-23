/** @file
 * @brief Check original observation replay, unknown ignored inputs, stale captures and independent fault domains.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "requests.c"
/** Traversal records complete membership but never reads ignored version bodies;
 * a separate common catalog replays the sealed image with zero physical I/O. */
static void it_replays_original_catalog_observations_without_reading_ignored_bodies_or_using_disk_after_seal(void) {
    for (unsigned standard = 0; standard < 2; standard++) {
        prepare_scope(standard != 0, standard != 0);
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK && observed.valid && observed.sealed);
        const SourceCatalogImageEntry *ignored = observed_entry(&observed, "/dependencies/vendor/pkg/v1.0.0");
        assert(ignored && !ignored->metadata_captured && !ignored->content && !ignored->children_complete);
        assert(!observed_entry(&observed, "/dependencies/vendor/pkg/v1.0.0/src/ignored"));
        ModuleCatalog replay; module_catalog_init(&replay, sources.allocator);
        fixture_io_begin(0);
        assert(module_catalog_build_view(&replay, observed.image.roots, observed.image.root_count, &observed.image.reader) == MODULE_OK);
        for (const ModuleSource *source = replay.first; source; source = source->next) {
            assert(module_catalog_load_configuration(&replay, source) == MODULE_OK);
            assert(module_catalog_load_native(&replay, source) == MODULE_OK);
        }
        assert(fixture_io_end() == 0); compare_catalog(&capture.catalog, &replay);
        size_t before = allocation_count;
        assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_INVALID && observed.valid);
        assert(allocation_count == before);
        const SourceText *text = NULL; struct stat metadata;
        fixture_io_begin(0);
        assert(observed.reader.read(&observed, source_path(capture.catalog.first->text), &text) == SOURCE_INVALID_ARGUMENT);
        assert(observed.reader.tree.root(&observed, &observed.catalog, observed.image.roots[0].path, &metadata) == MODULE_INVALID_PATH);
        assert(fixture_io_end() == 0 && observed.valid && observed.status == SOURCE_OBSERVATION_OK);
        module_catalog_destroy(&replay); source_observed_image_destroy(&observed); finish();
    }
}
/** Unknown-suffix companions and invalid-UTF8/NUL native bytes are captured before
 * original admission, then preserved exactly without native execution or validation. */
static void it_captures_every_original_opaque_companion_before_sealing_the_image(void) {
    setup_test(); selection_fixtures();
    const unsigned char bytes[] = {0xff, 0, '\r', '\n', 1};
    const char *path = "generated/project/packages/vendor/pkg/v1.0.0/src/parent.c";
    FILE *stream = fopen(path, "wb"); assert(stream);
    assert(fwrite(bytes, 1, sizeof(bytes), stream) == sizeof(bytes) && fclose(stream) == 0);
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent.unknown suffix", "opaque extra bytes\n");
    selection_capture_scope(false, false);
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    const SourceCatalogImageEntry *body = observed_entry(&observed, "/src/parent.c");
    const SourceCatalogImageEntry *extra = observed_entry(&observed, "/src/parent.unknown suffix");
    assert(body && body->metadata_captured && body->content && source_length(body->content) == sizeof(bytes));
    assert(!memcmp(source_bytes(body->content), bytes, sizeof(bytes)) && extra && extra->content);
    source_observed_image_destroy(&observed); finish();
}
/** Body, module-set, package, optional-alias and companion changes are never
 * adopted as a new baseline. Original captured bytes remain unchanged on refusal. */
static void it_refuses_changed_original_sources_versions_aliases_and_companion_membership(void) {
    for (unsigned profile = 0; profile < 7; profile++) {
        prepare();
        if (profile == 0) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child", "@!\nchanged\n"); }
        if (profile == 1) { file("generated/project/packages/vendor/pkg/v1.0.0/src/new module", "@!\n"); }
        if (profile == 2) { file("generated/project/packages/vendor/pkg/v1.0.0/package.ini", "; changed package\n"); }
        if (profile == 3) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent.ini", "; changed aliases\n"); }
        if (profile == 4) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent.new companion", "new\n"); }
        if (profile == 5) { assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parent.ini") == 0); }
        if (profile == 6) {
            assert(rename("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child",
                "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child.saved") == 0);
            file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child", "@!\n");
        }
        const ModuleSource *original = NULL;
        assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/parent/child", &original) == MODULE_OK);
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_CHANGED && !observed.valid && !observed.image.valid);
        assert(observed.failed_path && source_length(original->text) == 3 && !memcmp(source_bytes(original->text), "@!\n", 3));
        source_observed_image_destroy(&observed); finish();
    }
}
/** Fail every measured owner allocation while the original capture remains a
 * separately healthy immutable owner; every prefix returns to that baseline. */
static void it_retires_every_original_observation_allocation_failure(void) {
    prepare(); size_t allocations = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceObservationStatus status = source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0);
        if (!failure) { assert(status == SOURCE_OBSERVATION_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_OBSERVATION_MEMORY && !observed.valid && !observed.image.valid); }
        failure_at = 0; source_observed_image_destroy(&observed); assert(live_allocations == baseline);
    }
    finish();
}
/** Every physical metadata/directory acquisition, iteration and close failure
 * refuses preparation and closes all actual streams; no recapture is attempted. */
static void it_retires_every_original_observation_physical_io_failure(void) {
    prepare(); size_t calls = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= calls; failure++) {
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        fixture_io_begin(failure);
        SourceObservationStatus status = source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0);
        size_t measured = fixture_io_end();
        if (!failure) { assert(status == SOURCE_OBSERVATION_OK); calls = measured; assert(calls); }
        else { assert(status != SOURCE_OBSERVATION_OK && !observed.valid && !observed.image.valid); }
        source_observed_image_destroy(&observed); assert(live_allocations == baseline);
    }
    finish();
}
/** Invalid original ownership refuses before any physical work; an exactly empty
 * original domain seals an empty finite image without fabricated roots or modules. */
static void it_checks_observation_original_interfaces_and_accepts_an_empty_domain(void) {
    setup_test(); SourceRootCapture empty; source_root_capture_init(&empty, sources.allocator);
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(NULL, &empty, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_INVALID);
    assert(source_observed_image_build(&observed, &empty, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_INVALID && !observed.valid);
    source_observed_image_destroy(&observed);
    assert(source_root_capture_build(&empty, NULL, 0) == SOURCE_CAPTURE_OK);
    source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &empty, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK && observed.valid && !observed.image.count);
    source_observed_image_destroy(&observed);
    source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &empty, (SourceImageBounds){0}, NULL, 1) == SOURCE_OBSERVATION_INVALID && !observed.valid);
    source_observed_image_destroy(&observed); source_root_capture_destroy(&empty); destroy_test();
}
/** Exercise original replay, opaque input, contradiction and independent fault domains. */
void run(void) {
    it_observes_explicit_deep_inputs_and_replays_them_without_disk_fallback();
    it_refuses_invalid_or_unavailable_explicit_observation_demands();
    it_retires_every_requested_observation_allocation_and_physical_failure();
    it_replays_original_catalog_observations_without_reading_ignored_bodies_or_using_disk_after_seal();
    it_captures_every_original_opaque_companion_before_sealing_the_image();
    it_refuses_changed_original_sources_versions_aliases_and_companion_membership();
    it_retires_every_original_observation_allocation_failure();
    it_retires_every_original_observation_physical_io_failure();
    it_checks_observation_original_interfaces_and_accepts_an_empty_domain();
}
/** All sealed image borrowers expire before original capture teardown. */
int main(void) { run(); return 0; }
