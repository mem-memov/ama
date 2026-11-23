/** @file
 * @brief Verify simultaneous semantic-root replacement within a fixed physical envelope.
 * @see [T282 contract](../../../../../../docs/implementation/source-operations.md#t282-entry-review-and-execution)
 */
/** One projection may remove the old semantic root, but cannot remove its
 * physical observation boundary or silently restore the old root selection. */
static void it_assembles_root_replacement_in_one_projection_and_refuses_invalid_final_roots(void) {
    ImageAssemblyFixture fixture;
    image_assembly_setup(&fixture, "/image/from", "/image/new/child", false);
    fixture.root.path = "/image/directory";
    const char *boundary = "/image";
    SourceImageIssue shape;
    assert(source_catalog_image_prepare(&fixture.original, fixture.entries, 5, &fixture.root, 1,
        (SourceImageBounds){&boundary, 1}, &shape) == SOURCE_IMAGE_OK);
    SourceImageChange remove = {SOURCE_IMAGE_REMOVE, {.path = "/image/directory"}};
    ModuleRoot final_root = {.path = "/image/new", .kind = MODULE_ROOT_STANDARD};
    const SourceCatalogImage *output = &fixture.original;
    SourceCopyImageIssue issue;
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1,
        &final_root, 1, &output, &issue) == SOURCE_COPY_IMAGE_OK);
    assert(output->roots == &final_root && output->root_count == 1);
    assert(source_image_same_bounds(output, &fixture.original));
    assert(!image_assembly_entry(output, "/image/directory"));
    assert(image_assembly_entry(output, "/image/new/child")->content == fixture.entries[1].content);
    /* The previously used intermediate image really is invalid. */
    output = &fixture.original;
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1,
        &fixture.root, 1, &output, &issue) == SOURCE_COPY_IMAGE_PROJECTION);
    assert(output == &fixture.original && issue.projection_status == SOURCE_PROJECTION_SHAPE);
    const char *invalid[] = {"/image/missing", "/image/from", "/outside"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        final_root.path = invalid[i];
        assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1,
            &final_root, 1, &output, &issue) == SOURCE_COPY_IMAGE_PROJECTION);
        assert(output == &fixture.original && issue.projection_status == SOURCE_PROJECTION_SHAPE);
    }
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1,
        NULL, 1, &output, &issue) == SOURCE_COPY_IMAGE_INVALID && output == &fixture.original);
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1,
        NULL, 0, &output, &issue) == SOURCE_COPY_IMAGE_OK);
    assert(output->root_count == 0 && source_image_same_bounds(output, &fixture.original));
    assert(image_assembly_entry(output, "/image/new/child") && !image_assembly_entry(output, "/image/directory"));
    image_assembly_destroy(&fixture);
}
/** Exhaust actual root-changing preparation allocations; no failed prefix may
 * publish a final image or escape the scratch owner's teardown. */
static void it_retires_every_simultaneous_root_replacement_allocation_failure(void) {
    ImageAssemblyFixture fixture;
    image_assembly_setup(&fixture, "/image/from", "/image/new/child", false);
    fixture.root.path = "/image/directory";
    const char *boundary = "/image";
    SourceImageIssue shape;
    assert(source_catalog_image_prepare(&fixture.original, fixture.entries, 5, &fixture.root, 1,
        (SourceImageBounds){&boundary, 1}, &shape) == SOURCE_IMAGE_OK);
    SourceImageChange remove = {SOURCE_IMAGE_REMOVE, {.path = "/image/directory"}};
    ModuleRoot final_root = {.path = "/image/new", .kind = MODULE_ROOT_STANDARD};
    size_t total = 0, live = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena attempt; source_arena_init(&attempt, sources.allocator);
        const SourceCatalogImage *output = &fixture.original; SourceCopyImageIssue issue;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceCopyImageStatus status = source_copy_image_prepare(&attempt, &fixture.original, &fixture.map,
            NULL, &remove, 1, &final_root, 1, &output, &issue);
        if (!failure) { assert(status == SOURCE_COPY_IMAGE_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_COPY_IMAGE_OK && output == &fixture.original); }
        failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == live);
    }
    image_assembly_destroy(&fixture);
}
