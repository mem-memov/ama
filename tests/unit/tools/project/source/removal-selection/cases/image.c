/** @file
 * @brief Compose reviewed actual removal material with common final capture and compiler binding proof.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../../tools/project/source/graph/bindings.h"
/** Removed originals disappear; all retained captured inputs preserve exact names,
 * physical paths and bytes and the independently compiled final graph is unchanged. */
static void it_prepares_actual_removals_through_final_compiler_and_surviving_binding_checks(void) {
    for (unsigned profile = 0; profile < 4; profile++) {
        FixtureMaterial material = profile == 2 ? MATERIAL_OPAQUE : MATERIAL_NONE;
        begin_removal(material, profile == 1 ? subtree_path : profile == 3 ?
            "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty" : main_path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
        RemovalPlanFixture fixture;
        assert(removal_plan_fixture_build(&fixture, &removal) && fixture.plan.prepared);
        SourceOperationPlan *image = &fixture.plan;
        size_t expected = 0;
        for (const SourceCapturedUnit *old = capture.units; old; old = old->next) {
            bool removed = false;
            for (size_t i = 0; i < removal.identity_count; i++) { removed |= !strcmp(old->source->logical_path, removal.identities[i]); }
            const ModuleSource *now = NULL;
            assert(module_catalog_find(&image->capture.catalog, old->source->logical_path, &now) == (removed ? MODULE_MISSING : MODULE_OK));
            if (!removed) {
                expected++;
                assert(!strcmp(source_path(old->source->text), source_path(now->text)));
                assert(source_length(old->source->text) == source_length(now->text));
                assert(!memcmp(source_bytes(old->source->text), source_bytes(now->text), source_length(now->text)));
            }
        }
        assert(image->capture.view.count == expected);
        FILE *request = fopen("generated/final-removal.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &image->capture.view) && fclose(request) == 0);
        SourceCompilerClient final; source_compiler_client_init(&final, sources.allocator);
        assert(source_compiler_client_run(&final, "../../../../../../ama", "generated/final-removal.json", &image->capture.view, NULL) == SOURCE_COMPILER_OK);
        SourceCopyProofIssue issue;
        assert(source_operation_plan_prove(image, &final, &issue) == SOURCE_COPY_PROOF_OK);
        assert(source_operation_plan_prove(image, &final, &issue) == SOURCE_COPY_PROOF_OK);
        assert(source_operation_plan_prove(image, &analysis.original, &issue) ==
            (profile == 3 ? SOURCE_COPY_PROOF_OK : SOURCE_COPY_PROOF_INPUTS));
        assert(source_operation_plan_prove(NULL, &final, &issue) == SOURCE_COPY_PROOF_INVALID);
        assert(source_operation_plan_prove(image, NULL, &issue) == SOURCE_COPY_PROOF_INVALID);
        assert(source_operation_plan_prove(image, &final, NULL) == SOURCE_COPY_PROOF_INVALID);
        if (!profile) { removal_plan_proof_failures(image, &final); }
        source_compiler_client_destroy(&final);
        size_t before = allocation_count;
        assert(source_operation_plan_build(image, &image->inputs) == SOURCE_OPERATION_PLAN_INVALID && image->prepared && allocation_count == before);
        assert(access(main_path, F_OK) == 0 && access(subtree_path, F_OK) == 0);
        removal_plan_fixture_destroy(&fixture); end_removal(material);
    }
}
/** Original outside edges block before final image assembly. Changed raw data or new
 * children under removed directories refuse without silently expanding removal. */
static void it_refuses_blocked_changed_and_structurally_expanded_removal_images(void) {
    for (unsigned profile = 0; profile < 3; profile++) {
        FixtureMaterial material = profile == 0 ? MATERIAL_TESTS : MATERIAL_NONE;
        begin_removal(material, main_path);
        assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
        if (profile == 1) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/opaque.data", "changed data\n"); }
        if (profile == 2) { file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/new.data", "new opaque child\n"); }
        RemovalPlanFixture fixture;
        assert(!removal_plan_fixture_build(&fixture, &removal) && !fixture.plan.prepared);
        SourceOperationPlan *image = &fixture.plan;
        assert(image->status == (profile < 2 ? SOURCE_OPERATION_PLAN_REMOVAL : SOURCE_OPERATION_PLAN_IMAGE));
        if (profile == 0) { assert(image->removal_review.consumers); }
        else { assert(!image->capture.attempted); }
        removal_plan_fixture_destroy(&fixture); end_removal(material);
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
        RemovalPlanFixture fixture;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        bool prepared = removal_plan_fixture_build(&fixture, &removal);
        if (!failure) { assert(prepared); total = allocation_count - before; assert(total); }
        else { assert(!prepared && !fixture.plan.prepared); }
        failure_at = 0; removal_plan_fixture_destroy(&fixture); assert(live_allocations == baseline);
    }
    SourceOperationPlan image; source_operation_plan_init(&image, sources.allocator);
    assert(source_operation_plan_build(NULL, NULL) == SOURCE_OPERATION_PLAN_INVALID);
    assert(source_operation_plan_build(&image, NULL) == SOURCE_OPERATION_PLAN_INVALID && !image.prepared);
    source_operation_plan_destroy(&image); end_removal(MATERIAL_NONE);
}
