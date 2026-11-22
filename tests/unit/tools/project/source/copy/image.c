/** @file
 * @brief Verify general image scaffolding, exact replacement and preparation failure boundaries.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Fixed observed classes force new symbolic classes to avoid actual device-zero entries. */
typedef struct {
    SourceArena arena;
    SourceFileMap map;
    SourceCatalogImage original;
    SourceCatalogImageEntry entries[5];
    ModuleRoot root;
} ImageAssemblyFixture;
/** Observe fixture modes through ordinary test inputs, then give the immutable
 * model exact distinct identities and complete directory membership. */
static void image_assembly_setup(ImageAssemblyFixture *fixture, const char *source, const char *target, bool directory_input) {
    setup_test(); memset(fixture, 0, sizeof(*fixture));
    source_arena_init(&fixture->arena, sources.allocator); source_file_map_init(&fixture->map, sources.allocator);
    struct stat directory_mode, file_mode;
    assert(stat(".", &directory_mode) == 0 && stat("main.c", &file_mode) == 0);
    const char *paths[] = {"/image", "/image/from", "/image/target", "/image/blocked", "/image/directory"};
    for (size_t i = 0; i < 5; i++) {
        bool directory = i == 0 || i == 4;
        struct stat metadata = directory ? directory_mode : file_mode; metadata.st_dev = 0; metadata.st_ino = (ino_t)(i + 1);
        const SourceText *content = directory ? NULL : material_text(paths[i], (const unsigned char *)(i == 1 ? "from" : "old"), i == 1 ? 4 : 3);
        fixture->entries[i] = (SourceCatalogImageEntry){paths[i], metadata, content, true, directory};
    }
    fixture->root = (ModuleRoot){.path = "/image", .kind = MODULE_ROOT_PROJECT}; SourceImageIssue issue;
    assert(source_catalog_image_prepare(&fixture->original, fixture->entries, 5, &fixture->root, 1, &issue) == SOURCE_IMAGE_OK);
    const SourceFileMapping *row = NULL;
    assert(source_file_map_observation(&fixture->map, source, directory_input ? NULL : fixture->entries[1].content,
        directory_input, target, &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&fixture->map) == SOURCE_FILE_MAP_OK);
}
/** Release prepared image metadata before its captured source/map owners. */
static void image_assembly_destroy(ImageAssemblyFixture *fixture) {
    source_arena_destroy(&fixture->arena); source_file_map_destroy(&fixture->map); destroy_test();
}
/** Exact output lookup keeps assertions independent of result row ordering. */
static const SourceCatalogImageEntry *image_assembly_entry(const SourceCatalogImage *image, const char *path) {
    for (size_t i = 0; i < image->count; i++) { if (!strcmp(image->entries[i].path, path)) { return &image->entries[i]; } }
    return NULL;
}
/** New ancestors and content retain originals and distinct physical classes;
 * an explicit removal allows file/file or file/directory replacement exactly once. */
static void it_assembles_exact_ancestor_images_and_explicit_replacements_without_host_mutation(void) {
    ImageAssemblyFixture fixture; image_assembly_setup(&fixture, "/image/from", "/image/new/deep/copied", false);
    const SourceCatalogImage *output = NULL; SourceCopyImageIssue issue;
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, NULL, 0, &output, &issue) == SOURCE_COPY_IMAGE_OK);
    assert(output->count == 8 && output->reader.tree.context == output);
    assert(image_assembly_entry(output, "/image/new") && image_assembly_entry(output, "/image/new/deep"));
    assert(image_assembly_entry(output, "/image/new/deep/copied")->content == fixture.entries[1].content);
    for (size_t i = 0; i < output->count; i++) {
        for (size_t j = 0; j < i; j++) { assert(output->entries[i].metadata.st_dev != output->entries[j].metadata.st_dev
            || output->entries[i].metadata.st_ino != output->entries[j].metadata.st_ino); }
    }
    image_assembly_destroy(&fixture);
    for (unsigned directory_input = 0; directory_input < 2; directory_input++) {
        image_assembly_setup(&fixture, directory_input ? "/image/directory" : "/image/from", "/image/target", directory_input != 0);
        SourceImageChange remove = {SOURCE_IMAGE_REMOVE, {.path = "/image/target"}};
        assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1, &output, &issue) == SOURCE_COPY_IMAGE_OK);
        assert(output->count == 5);
        const SourceCatalogImageEntry *target = image_assembly_entry(output, "/image/target");
        assert(target && (directory_input ? S_ISDIR(target->metadata.st_mode) && target->children_complete : target->content == fixture.entries[1].content));
        assert(target->metadata.st_ino != fixture.entries[2].metadata.st_ino);
        image_assembly_destroy(&fixture);
    }
    image_assembly_setup(&fixture, "/image/from", "/image/target", false);
    source_file_map_destroy(&fixture.map); source_file_map_init(&fixture.map, sources.allocator);
    assert(source_file_map_seal(&fixture.map) == SOURCE_FILE_MAP_OK);
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, NULL, 0, &output, &issue) == SOURCE_COPY_IMAGE_OK);
    assert(output->count == 5);
    SourceImageChange remove = {SOURCE_IMAGE_REMOVE, {.path = "/image/target"}};
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &remove, 1, &output, &issue) == SOURCE_COPY_IMAGE_OK);
    assert(output->count == 4 && !image_assembly_entry(output, "/image/target") && image_assembly_entry(output, "/image/from"));
    image_assembly_destroy(&fixture);
}
/** Existing/unrelated/unknown paths, duplicate supplemental keys and incomplete
 * original membership refuse while every partial scratch allocation stays owned. */
static void it_refuses_ambiguous_unobserved_and_outside_image_assembly_and_retires_failures(void) {
    const char *targets[] = {"/image/target", "/image/blocked/child", "/outside/copied", "/image/new/copied", "/image/new/copied"};
    const SourceCopyImageStatus expected[] = {SOURCE_COPY_IMAGE_COLLISION, SOURCE_COPY_IMAGE_ORIGINAL,
        SOURCE_COPY_IMAGE_SCOPE, SOURCE_COPY_IMAGE_ORIGINAL, SOURCE_COPY_IMAGE_ORIGINAL};
    for (unsigned profile = 0; profile < 5; profile++) {
        ImageAssemblyFixture fixture; image_assembly_setup(&fixture, profile == 3 ? "/image/absent" : "/image/from", targets[profile], false);
        if (profile == 4) { fixture.entries[0].children_complete = false; }
        const SourceCatalogImage *output = &fixture.original; SourceCopyImageIssue issue;
        assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, NULL, 0, &output, &issue) == expected[profile]);
        assert(output == &fixture.original && issue.path); image_assembly_destroy(&fixture);
    }
    ImageAssemblyFixture fixture; image_assembly_setup(&fixture, "/image/from", "/image/new/deep/copied", false);
    SourceImageChange duplicated[] = {{SOURCE_IMAGE_REMOVE, {.path = "/image/target"}}, {SOURCE_IMAGE_REMOVE, {.path = "/image/target"}}};
    const SourceCatalogImage *output = &fixture.original; SourceCopyImageIssue issue;
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, duplicated, 2, &output, &issue) == SOURCE_COPY_IMAGE_COLLISION);
    assert(output == &fixture.original);
    SourceImageChange invalid = {SOURCE_IMAGE_REMOVE, {.path = "/image/not observed"}};
    assert(source_copy_image_prepare(&fixture.arena, &fixture.original, &fixture.map, NULL, &invalid, 1, &output, &issue) == SOURCE_COPY_IMAGE_PROJECTION);
    assert(output == &fixture.original && issue.projection_status == SOURCE_PROJECTION_MISSING);
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena attempt; source_arena_init(&attempt, sources.allocator); output = &fixture.original;
        size_t start = allocation_count; if (failure) { failure_at = start + failure; }
        SourceCopyImageStatus status = source_copy_image_prepare(&attempt, &fixture.original, &fixture.map, NULL, NULL, 0, &output, &issue);
        if (!failure) { assert(status == SOURCE_COPY_IMAGE_OK); total = allocation_count - start; assert(total); }
        else {
            assert(status == SOURCE_COPY_IMAGE_MEMORY
                || (status == SOURCE_COPY_IMAGE_MATERIAL && issue.material_status == SOURCE_MATERIAL_MEMORY)
                || (status == SOURCE_COPY_IMAGE_PROJECTION && issue.projection_status == SOURCE_PROJECTION_ALLOCATION));
            assert(output == &fixture.original);
        }
        failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == baseline);
    }
    image_assembly_destroy(&fixture);
}
