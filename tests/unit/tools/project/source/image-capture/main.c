/** @file
 * @brief Check prepared-image compiler capture ownership, unavailable units and independent compiler admission.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../image/common.c"
#include "../../../../../../tools/project/source/capture/image.h"
#include "../../../../../../tools/project/source/compiler/client.h"
#include "delivery/source/snapshot/write.h"
/** The complete capture fixture has bytes for both opaque native inputs; the
 * independent image-only fixture intentionally leaves those unread by default. */
static void capture_rows(const ViewModel *model, SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]) {
    image_rows(model, rows); rows[12].content = model->texts[12]; rows[13].content = model->texts[13];
}
/** The common final catalog and unit capture own new-path text, then the existing
 * process protocol independently parses/checks the captured complete domain. */
static void it_captures_complete_virtual_image_inputs_and_admits_the_shared_snapshot_protocol(void) {
    setup_test(); ViewModel model; model_init(&model); ModuleCatalogReader oracle = reader_for(&model);
    ModuleCatalog expected; module_catalog_init(&expected, sources.allocator);
    assert(prepare_catalog(&expected, &oracle) == MODULE_OK);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; capture_rows(&model, rows);
    SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
    assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 1, image_roots, 2, &issue) == SOURCE_IMAGE_OK);
    SourceImageCapture capture; source_image_capture_init(&capture, sources.allocator);
    fixture_io_begin(0);
    assert(source_image_capture_build(&capture, &image) == SOURCE_IMAGE_CAPTURE_OK && capture.valid);
    assert(fixture_io_end() == 0); same_catalog(&expected, &capture.catalog);
    assert(capture.view.count == 4 && capture.view.versions && !capture.view.versions->next);
    const DeliverySourceCompanions *native = capture.view.companions;
    while (native && strcmp(native->identity, "/vendor/pkg/v1.0.0/src/state")) { native = native->next; }
    assert(native && !strcmp(native->body, nodes[12].path) && !strcmp(native->header, nodes[13].path));
    assert(native->abi && !strcmp(source_path(native->abi), nodes[14].path));
    size_t before = allocation_count;
    assert(source_image_capture_build(&capture, &image) == SOURCE_IMAGE_CAPTURE_INVALID && capture.valid);
    assert(allocation_count == before);
    FILE *request = fopen("generated/image-request.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &capture.view) && fclose(request) == 0);
    SourceCompilerClient compiler; source_compiler_client_init(&compiler, sources.allocator);
    SourceCompilerStatus status = source_compiler_client_run(&compiler, "../../../../../../ama",
        "generated/image-request.json", &capture.view, NULL);
    if (status != SOURCE_COMPILER_OK && compiler.diagnostics) {
        fwrite(source_bytes(compiler.diagnostics), 1, source_length(compiler.diagnostics), stderr);
    }
    assert(status == SOURCE_COMPILER_OK && compiler.valid);
    source_compiler_client_destroy(&compiler); source_image_capture_destroy(&capture);
    module_catalog_destroy(&expected); destroy_test();
}
/** Unit capture sees unavailable companions and wrong kinds even when an ordinary
 * body does not activate native C. It never reinterprets them as absent inputs. */
static void it_refuses_unavailable_and_nonregular_image_companions_without_disk_fallback(void) {
    for (unsigned profile = 0; profile < 4; profile++) {
        setup_test(); ViewModel model; model_init(&model);
        SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; capture_rows(&model, rows);
        if (profile == 0) { rows[12].content = NULL; }
        if (profile == 1) { rows[12].metadata_captured = false; rows[12].content = NULL; }
        if (profile == 2) { rows[12].metadata = model.directory; rows[12].content = NULL; rows[12].children_complete = true; }
        if (profile == 3) { rows[5].children_complete = false; }
        SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
        assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 1, image_roots, 2, &issue) == SOURCE_IMAGE_OK);
        SourceImageCapture capture; source_image_capture_init(&capture, sources.allocator);
        fixture_io_begin(0);
        SourceImageCaptureStatus status = source_image_capture_build(&capture, &image);
        assert(status == (profile == 2 ? SOURCE_IMAGE_CAPTURE_KIND : profile == 3 ? SOURCE_IMAGE_CAPTURE_CATALOG : SOURCE_IMAGE_CAPTURE_UNAVAILABLE));
        assert(!capture.valid && !capture.view.entries && fixture_io_end() == 0);
        assert(capture.failed_path);
        source_image_capture_destroy(&capture); destroy_test();
    }
}
/** Every measured catalog/unit/table/owned-string allocation can independently
 * refuse with no source I/O or partially accepted public snapshot. */
static void it_retires_every_prepared_image_capture_allocation_failure(void) {
    setup_test(); ViewModel model; model_init(&model);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; capture_rows(&model, rows);
    SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
    assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 1, image_roots, 2, &issue) == SOURCE_IMAGE_OK);
    size_t allocations = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        SourceImageCapture capture; source_image_capture_init(&capture, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        fixture_io_begin(0);
        SourceImageCaptureStatus status = source_image_capture_build(&capture, &image);
        assert(fixture_io_end() == 0);
        if (!failure) { assert(status == SOURCE_IMAGE_CAPTURE_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_IMAGE_CAPTURE_MEMORY && !capture.valid && !capture.view.entries); }
        failure_at = 0; source_image_capture_destroy(&capture); assert(live_allocations == baseline);
    }
    destroy_test();
}
/** Unknown dot suffixes are complete unit inputs, while similarly prefixed
 * orphan assets do not join another main's compiler-input unit. */
static void it_preserves_unknown_binary_companions_and_ignores_other_module_stems(void) {
    setup_test(); ViewModel model; model_init(&model);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 3]; capture_rows(&model, rows);
    rows[VIEW_NODE_COUNT + 1] = (SourceCatalogImageEntry){.path = "/virtual/packages/vendor/pkg/v1.0.0/src/state.extra",
        .metadata = model.regular, .metadata_captured = true, .content = model.texts[12]};
    rows[VIEW_NODE_COUNT + 1].metadata.st_ino = 1000;
    rows[VIEW_NODE_COUNT + 2] = (SourceCatalogImageEntry){.path = "/virtual/packages/vendor/pkg/v1.0.0/src/stateful.c"};
    SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
    assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 3, image_roots, 2, &issue) == SOURCE_IMAGE_OK);
    SourceImageCapture capture; source_image_capture_init(&capture, sources.allocator);
    fixture_io_begin(0);
    assert(source_image_capture_build(&capture, &image) == SOURCE_IMAGE_CAPTURE_OK && fixture_io_end() == 0);
    const SourceCapturedUnit *unit = capture.units;
    while (unit && strcmp(unit->source->logical_path, "/vendor/pkg/v1.0.0/src/state")) { unit = unit->next; }
    assert(unit); const AcquisitionUnitEntry *extra = NULL; const char *previous = NULL;
    for (const AcquisitionUnitEntry *file = unit->files; file; file = file->next) {
        assert(!previous || strcmp(previous, file->file.path) < 0); previous = file->file.path;
        assert(!strstr(file->file.path, "stateful.c"));
        if (!strcmp(file->file.path, rows[VIEW_NODE_COUNT + 1].path)) { extra = file; }
    }
    assert(extra && source_length(extra->file.content) == source_length(model.texts[12]));
    assert(!memcmp(source_bytes(extra->file.content), source_bytes(model.texts[12]), source_length(model.texts[12])));
    assert(!strcmp(source_path(extra->file.content), rows[VIEW_NODE_COUNT + 1].path));
    source_image_capture_destroy(&capture); destroy_test();
}
/** No physical-original authority is fabricated for empty or invalid images. */
static void it_checks_prepared_capture_interfaces_and_accepts_an_empty_image(void) {
    setup_test(); SourceImageCapture capture; source_image_capture_init(&capture, sources.allocator);
    SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
    assert(source_image_capture_build(NULL, &image) == SOURCE_IMAGE_CAPTURE_INVALID);
    assert(source_image_capture_build(&capture, &image) == SOURCE_IMAGE_CAPTURE_INVALID && !capture.valid);
    source_image_capture_destroy(&capture); source_image_capture_init(&capture, sources.allocator);
    assert(source_catalog_image_prepare(&image, NULL, 0, NULL, 0, &issue) == SOURCE_IMAGE_OK);
    assert(source_image_capture_build(&capture, &image) == SOURCE_IMAGE_CAPTURE_OK && capture.valid);
    assert(!capture.view.count && !capture.view.entries && !capture.view.versions && !capture.view.companions);
    source_image_capture_destroy(&capture); destroy_test();
}
/** Run common discovery, process handoff, complete unit/fault and ownership boundaries. */
void run(void) {
    it_captures_complete_virtual_image_inputs_and_admits_the_shared_snapshot_protocol();
    it_refuses_unavailable_and_nonregular_image_companions_without_disk_fallback();
    it_retires_every_prepared_image_capture_allocation_failure();
    it_preserves_unknown_binary_companions_and_ignores_other_module_stems();
    it_checks_prepared_capture_interfaces_and_accepts_an_empty_image();
}
/** Retire every snapshot/catalog borrower before the source image's input owners. */
int main(void) { run(); return 0; }
