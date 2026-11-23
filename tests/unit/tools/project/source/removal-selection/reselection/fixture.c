/** @file
 * @brief Freeze selected local and ignored external versions before genuine removal/reselection tests.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
/** Own all original/final borrowers through independent compiler and proof checks. */
typedef struct {
    SourceArena arena;
    SourceObservedImage observed;
    SourceCloneMap mains;
    SourceFileMap files;
    SourceImageCapture final;
    SourceCompilerClient client;
    SourceRemovalMaterial material;
    SourceObservationRequest *requests;
    size_t request_count;
    const char *external;
    ModuleRoot final_root;
} ReselectionFixture;
/** Create an outside surviving definition with either no edges or a genuine
 * permitted edge to the version which will be removed. */
static void reselection_original(ReselectionFixture *fixture, unsigned profile) {
    bool consumer = (profile & 1) != 0, excluded = (profile & 2) != 0;
    setup_test(); memset(fixture, 0, sizeof(*fixture)); source_arena_init(&fixture->arena, sources.allocator);
    const char *dirs[] = {"generated/fallback", "generated/fallback/packages", "generated/fallback/packages/vendor",
        "generated/fallback/packages/vendor/pkg", "generated/fallback/packages/vendor/pkg/v1.0.0",
        "generated/fallback/packages/vendor/pkg/v1.0.0/src", "generated/fallback/packages/vendor/pkg/v1.0.0/tests",
        "generated/fallback/dependencies", "generated/fallback/dependencies/vendor", "generated/fallback/dependencies/vendor/pkg",
        "generated/fallback/dependencies/vendor/pkg/v1.0.0", "generated/fallback/dependencies/vendor/pkg/v1.0.0/src",
        "generated/fallback/dependencies/vendor/pkg/v1.0.0/tests", "generated/fallback/packages/vendor/other",
        "generated/fallback/packages/vendor/other/v1.0.0", "generated/fallback/packages/vendor/other/v1.0.0/src",
        "generated/fallback/packages/vendor/other/v1.0.0/tests"};
    for (size_t i = 0; i < sizeof(dirs)/sizeof(*dirs); i++) { directory(dirs[i]); }
    file("generated/fallback/packages/vendor/pkg/v1.0.0/package.ini", "");
    file("generated/fallback/packages/vendor/pkg/v1.0.0/src/main", "@!\n# local\n");
    file("generated/fallback/packages/vendor/pkg/v1.0.0/tests/check", "@!\nitem&\n/vendor/pkg/v1.0.0/src/main\n");
    file("generated/fallback/dependencies/vendor/pkg/v1.0.0/package.ini", "");
    file("generated/fallback/dependencies/vendor/pkg/v1.0.0/src/main", "@!\n# external\n");
    file("generated/fallback/dependencies/vendor/pkg/v1.0.0/src/main.extra", "opaque companion\n");
    file("generated/fallback/dependencies/vendor/pkg/v1.0.0/tests/check", "@!\nitem&\n/vendor/pkg/v1.0.0/src/main\n");
    file("generated/fallback/packages/vendor/other/v1.0.0/package.ini", consumer ? "[dependencies]\n/vendor/pkg/v1.0.0/src/main\n" : "");
    file("generated/fallback/packages/vendor/other/v1.0.0/src/main", consumer ? "@!\nitem&\n/vendor/pkg/v1.0.0/src/main\n" : "@!\n# retained\n");
    file("generated/fallback/dependencies/vendor/pkg/v1.0.0/src/extra", "@!\n# only external\n");
    file("generated/fallback/dependencies/vendor/pkg/v1.0.0/tests/extra", "@!\n");
    source_root_capture_init(&capture, sources.allocator);
    const char *exclusions[] = {"dependencies/vendor/pkg/v1.0.0"};
    ModuleRoot root = {"generated/fallback", excluded ? exclusions : NULL, excluded ? 1u : 0u, MODULE_ROOT_PROJECT};
    assert(source_root_capture_build(&capture, &root, 1) == SOURCE_CAPTURE_OK);
    fixture->final_root = (ModuleRoot){capture.roots[0].path, NULL, 0, MODULE_ROOT_PROJECT};
    source_tree_selection_init(&selection, sources.allocator); source_analysis_init(&analysis, sources.allocator);
    FILE *request = fopen("generated/fallback-original.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &capture.view) && fclose(request) == 0);
    assert(source_analysis_check(&analysis, &capture.view, "../../../../../../ama", "generated/fallback-original.json") == SOURCE_ANALYSIS_OK);
    assert(source_tree_selection_read(&selection, &capture, "generated/fallback/packages/vendor/pkg/v1.0.0") == SOURCE_TREE_OK);
    source_removal_selection_init(&removal, sources.allocator);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    assert(source_removal_material_prepare(&fixture->arena, &removal, &fixture->material) == SOURCE_REMOVAL_MATERIAL_OK);
    source_observed_image_init(&fixture->observed, sources.allocator);
    source_clone_map_init(&fixture->mains, sources.allocator, &capture); source_file_map_init(&fixture->files, sources.allocator);
    assert(source_clone_map_seal(&fixture->mains) == SOURCE_CLONE_OK && source_file_map_seal(&fixture->files) == SOURCE_FILE_MAP_OK);
    source_image_capture_init(&fixture->final, sources.allocator); source_compiler_client_init(&fixture->client, sources.allocator);
}
/** Retire final compiler/capture before their original material/map owners. */
static void reselection_destroy(ReselectionFixture *fixture) {
    source_compiler_client_destroy(&fixture->client); source_image_capture_destroy(&fixture->final);
    source_file_map_destroy(&fixture->files); source_clone_map_destroy(&fixture->mains);
    source_observed_image_destroy(&fixture->observed); source_arena_destroy(&fixture->arena);
    source_removal_selection_destroy(&removal); source_analysis_destroy(&analysis); finish();
}
