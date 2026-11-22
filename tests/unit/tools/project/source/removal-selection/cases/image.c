/** @file
 * @brief Compose reviewed actual removal material with common final capture and compiler binding proof.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../../tools/project/source/removal/image/image.h"
#include "../../../../../../../tools/project/source/graph/bindings.h"
/** Removed originals disappear; all retained captured inputs preserve exact names,
 * physical paths and bytes and the independently compiled final graph is unchanged. */
static void it_prepares_actual_removals_through_final_compiler_and_surviving_binding_checks(void) {
    for (unsigned profile = 0; profile < 4; profile++) {
        FixtureMaterial material = profile == 2 ? MATERIAL_OPAQUE : MATERIAL_NONE;
        begin_removal(material, profile == 1 ? subtree_path : profile == 3 ?
            "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty" : main_path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
        SourceRemovalImage image; source_removal_image_init(&image, sources.allocator);
        assert(source_removal_image_build(&image, &removal, NULL, NULL) == SOURCE_REMOVAL_IMAGE_OK && image.valid);
        size_t expected = 0;
        for (const SourceCapturedUnit *old = capture.units; old; old = old->next) {
            bool removed = false;
            for (size_t i = 0; i < removal.identity_count; i++) { removed |= !strcmp(old->source->logical_path, removal.identities[i]); }
            const ModuleSource *now = NULL;
            assert(module_catalog_find(&image.capture.catalog, old->source->logical_path, &now) == (removed ? MODULE_MISSING : MODULE_OK));
            if (!removed) {
                expected++;
                assert(!strcmp(source_path(old->source->text), source_path(now->text)));
                assert(source_length(old->source->text) == source_length(now->text));
                assert(!memcmp(source_bytes(old->source->text), source_bytes(now->text), source_length(now->text)));
            }
        }
        assert(image.capture.view.count == expected);
        FILE *request = fopen("generated/final-removal.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &image.capture.view) && fclose(request) == 0);
        SourceCompilerClient final; source_compiler_client_init(&final, sources.allocator);
        assert(source_compiler_client_run(&final, "../../../../../../ama", "generated/final-removal.json", &image.capture.view, NULL) == SOURCE_COMPILER_OK);
        SourceSitesIssue issue; size_t proof_allocations = allocation_count;
        assert(source_removal_image_prove(&image, &final, &issue) == SOURCE_REMOVAL_PROOF_OK);
        assert(source_removal_image_prove(&image, &final, &issue) == SOURCE_REMOVAL_PROOF_OK);
        assert(source_removal_image_prove(&image, &analysis.original, &issue) ==
            (profile == 3 ? SOURCE_REMOVAL_PROOF_OK : SOURCE_REMOVAL_PROOF_INPUTS));
        assert(source_removal_image_prove(NULL, &final, &issue) == SOURCE_REMOVAL_PROOF_INVALID);
        assert(source_removal_image_prove(&image, NULL, &issue) == SOURCE_REMOVAL_PROOF_INVALID);
        assert(source_removal_image_prove(&image, &final, NULL) == SOURCE_REMOVAL_PROOF_INVALID);
        assert(allocation_count == proof_allocations);
        source_compiler_client_destroy(&final);
        size_t before = allocation_count;
        assert(source_removal_image_build(&image, &removal, NULL, NULL) == SOURCE_REMOVAL_IMAGE_INVALID && image.valid && allocation_count == before);
        assert(access(main_path, F_OK) == 0 && access(subtree_path, F_OK) == 0);
        source_removal_image_destroy(&image); end_removal(material);
    }
}
/** Original outside edges block before observation. Changed raw data or new
 * children under removed directories refuse without silently expanding removal. */
static void it_refuses_blocked_changed_and_structurally_expanded_removal_images(void) {
    for (unsigned profile = 0; profile < 3; profile++) {
        FixtureMaterial material = profile == 0 ? MATERIAL_TESTS : MATERIAL_NONE;
        begin_removal(material, main_path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
        if (profile == 1) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/opaque.data", "changed data\n"); }
        if (profile == 2) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/new.data", "new opaque child\n"); }
        SourceRemovalImage image; source_removal_image_init(&image, sources.allocator);
        SourceRemovalImageStatus expected = profile == 0 ? SOURCE_REMOVAL_IMAGE_REVIEW :
            profile == 1 ? SOURCE_REMOVAL_IMAGE_CHANGED : SOURCE_REMOVAL_IMAGE_PROJECTION;
        assert(source_removal_image_build(&image, &removal, NULL, NULL) == expected && !image.valid);
        if (profile == 0) { assert(image.review.consumers && !image.observed.attempted); }
        else { assert(image.failed_path && !image.capture.attempted); }
        source_removal_image_destroy(&image); end_removal(material);
        if (profile == 2) { assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/new.data") == 0); }
    }
}
/** Every measured preparation allocation retires; independent original owners and
 * all source files survive even when a late final-catalog allocation fails. */
static void it_retires_every_removal_image_allocation_failure_and_checks_invalid_inputs(void) {
    begin_removal(MATERIAL_NONE, main_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceRemovalImage image; source_removal_image_init(&image, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceRemovalImageStatus status = source_removal_image_build(&image, &removal, NULL, NULL);
        if (!failure) { assert(status == SOURCE_REMOVAL_IMAGE_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_REMOVAL_IMAGE_OK && !image.valid); }
        failure_at = 0; source_removal_image_destroy(&image); assert(live_allocations == baseline);
    }
    SourceRemovalImage image; source_removal_image_init(&image, sources.allocator);
    assert(source_removal_image_build(NULL, &removal, NULL, NULL) == SOURCE_REMOVAL_IMAGE_INVALID);
    assert(source_removal_image_build(&image, NULL, NULL, NULL) == SOURCE_REMOVAL_IMAGE_INVALID && !image.valid);
    source_removal_image_destroy(&image); end_removal(MATERIAL_NONE);
}
