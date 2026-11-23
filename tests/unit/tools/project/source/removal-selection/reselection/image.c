/** @file
 * @brief Prepare observed ignored material and independently compile its selected final removal image.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
/** Request exact ignored configuration, directories, mains and opaque companions;
 * ordinary catalog traversal deliberately does not read them while ignored. */
static void reselection_observe(ReselectionFixture *fixture) {
    fixture->external = source_arena_join(&fixture->arena, capture.roots[0].path, "/", "dependencies/vendor/pkg/v1.0.0"); assert(fixture->external);
    const struct { const char *suffix; SourceObservationKind kind; } extra[] = {
        {"", SOURCE_OBSERVE_CHILDREN}, {"/package.ini", SOURCE_OBSERVE_CONTENT},
        {"/src", SOURCE_OBSERVE_CHILDREN}, {"/tests", SOURCE_OBSERVE_CHILDREN},
        {"/src/main", SOURCE_OBSERVE_CONTENT}, {"/src/main.extra", SOURCE_OBSERVE_CONTENT},
        {"/tests/check", SOURCE_OBSERVE_CONTENT}, {"/src/extra", SOURCE_OBSERVE_CONTENT},
        {"/tests/extra", SOURCE_OBSERVE_CONTENT}};
    fixture->request_count = fixture->material.count + sizeof(extra)/sizeof(*extra);
    fixture->requests = source_arena_allocate(&fixture->arena, fixture->request_count * sizeof(*fixture->requests)); assert(fixture->requests);
    memcpy(fixture->requests, fixture->material.requests, fixture->material.count * sizeof(*fixture->requests));
    for (size_t i = 0; i < sizeof(extra)/sizeof(*extra); i++) {
        const char *path = source_arena_join(&fixture->arena, fixture->external, "", extra[i].suffix); assert(path);
        fixture->requests[fixture->material.count + i] = (SourceObservationRequest){path, extra[i].kind, false};
    }
    assert(source_observed_image_build(&fixture->observed, &capture, (SourceImageBounds){0},
        fixture->requests, fixture->request_count) == SOURCE_OBSERVATION_OK);
}
/** The real common selector and isolated compiler establish all newly selected
 * definitions; no synthetic source/capture validity flags participate. */
static void reselection_image(ReselectionFixture *fixture) {
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&fixture->arena, &fixture->observed.image, &fixture->files, NULL,
        fixture->material.changes, fixture->material.count, &fixture->final_root, 1, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    assert(source_image_capture_build(&fixture->final, image) == SOURCE_IMAGE_CAPTURE_OK);
    const ModuleSource *selected = NULL;
    assert(module_catalog_find(&fixture->final.catalog, "/vendor/pkg/v1.0.0/src/main", &selected) == MODULE_OK);
    assert(!strcmp(source_path(selected->text), source_arena_join(&fixture->arena, fixture->external, "/", "src/main")));
    FILE *request = fopen("generated/fallback-final.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &fixture->final.view) && fclose(request) == 0);
    assert(source_compiler_client_run(&fixture->client, "../../../../../../ama", "generated/fallback-final.json",
        &fixture->final.view, NULL) == SOURCE_COMPILER_OK);
}
