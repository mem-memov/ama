/** @file
 * @brief Compile changed supplier images and reject changed opaque/configuration bytes or new code provenance.
 * @see [T285 entry review](../../../../../../../docs/implementation/source-operations.md#t285-entry-review-and-execution)
 */
#include "../../../../../../../tools/project/source/copy/image/internal.h"
/** Final code can compile while its original supplier material is altered. All
 * modifications pass through the actual immutable projection/capture pipeline. */
static void it_rejects_changed_reselected_material_and_arbitrary_new_modules_even_when_they_compile(void) {
    ReselectionFixture fixture; reselection_original(&fixture, false); reselection_observe(&fixture); reselection_image(&fixture);
    const struct { const char *suffix; const char *bytes; SourceImageChangeKind kind; } variants[] = {
        {"/src/main", "@!\n# changed\n", SOURCE_IMAGE_UPDATE},
        {"/src/main.extra", "changed companion\n", SOURCE_IMAGE_UPDATE},
        {"/package.ini", "; changed configuration\n", SOURCE_IMAGE_UPDATE},
        {"/src/main.extra", NULL, SOURCE_IMAGE_REMOVE},
        {"/src/new", "@!\n", SOURCE_IMAGE_ADD}};
    for (size_t variant = 0; variant < sizeof(variants)/sizeof(*variants); variant++) {
        SourceImageChange change = {.kind = variants[variant].kind};
        const char *base = variant == 4 ? source_arena_join(&fixture.arena, capture.roots[0].path, "/", "packages/vendor/other/v1.0.0") : fixture.external;
        assert(base); const char *path = source_arena_join(&fixture.arena, base, "", variants[variant].suffix); assert(path);
        const SourceCatalogImage *before = fixture.final.image;
        for (size_t i = 0; i < before->count; i++) {
            if (!strcmp(before->entries[i].path, path)) { change.entry = before->entries[i]; break; }
        }
        if (change.kind == SOURCE_IMAGE_ADD) {
            SourceCopyImageBuilder builder = {.original = before};
            change.entry = (SourceCatalogImageEntry){.path = path, .metadata_captured = true};
            assert(stat("generated/fallback/dependencies/vendor/pkg/v1.0.0/src/main", &change.entry.metadata) == 0);
            assert(source_copy_image_identity(&builder, &change.entry.metadata) == SOURCE_COPY_IMAGE_OK);
        } else { assert(change.entry.path); }
        SourceManager texts; source_manager_init(&texts, sources.allocator);
        if (variants[variant].bytes) {
            assert(source_manager_add(&texts, path, (const unsigned char *)variants[variant].bytes,
                strlen(variants[variant].bytes), &change.entry.content) == SOURCE_OK);
        }
        SourceCatalogImage image; SourceProjectionIssue projection;
        assert(source_image_project(&fixture.arena, before, &change, 1, before->roots, before->root_count, &image, &projection) == SOURCE_PROJECTION_OK);
        SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
        assert(source_image_capture_build(&final, &image) == SOURCE_IMAGE_CAPTURE_OK);
        FILE *request = fopen("generated/fallback-changed.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
        SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
        assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/fallback-changed.json", &final.view, NULL) == SOURCE_COMPILER_OK);
        SourceCopyProofIssue issue;
        assert(source_copy_prove(&fixture.arena, &analysis, &fixture.observed, &fixture.mains, &fixture.files, NULL, NULL,
            &final, &client, removal.identities, removal.identity_count, &issue) == SOURCE_COPY_PROOF_INTRODUCTION);
        assert(issue.introduction_status == (variant == 4 ? SOURCE_INTRODUCTION_ORIGIN : SOURCE_INTRODUCTION_MATERIAL));
        source_compiler_client_destroy(&client); source_image_capture_destroy(&final); source_manager_destroy(&texts);
    }
    reselection_destroy(&fixture);
}
