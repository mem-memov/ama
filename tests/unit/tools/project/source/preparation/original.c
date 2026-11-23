/** @file
 * @brief Compose parsed copy/move root choices with genuine original augmentation before compiler analysis.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
/** O remains an independent captured authority; the compiler sees one U capture
 * including preexisting destination inputs, never a final-only catalog or merged
 * replies. This prepares original facts only, not complete copy/move support. */
static void it_analyzes_augmented_original_inputs_before_preparing_copy_or_move(void) {
    for (unsigned move = 0; move < 2; move++) {
        setup_test(); assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/additional", 0700) == 0 && mkdir("generated/scratch", 0700) == 0);
        file("generated/source/first", "@!\n"); file("generated/additional/second", "@!\n");
        const char *arguments[] = {"ama-project", move ? "move" : "copy", "generated/source", "generated/copied", "--preview",
            "--standard-root", "generated/source", "--destination-standard-root", "generated/additional", "--compiler", "../../../../../../ama"};
        ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
        assert(project_source_request_parse(&request, (int)(sizeof(arguments)/sizeof(*arguments)), arguments) == PROJECT_SOURCE_REQUEST_OK);
        ProjectSourcePreparation owner; project_source_preparation_init(&owner, sources.allocator);
        ProjectSourcePreparationInputs inputs = (ProjectSourcePreparationInputs){.request = &request, .temporary_parent = "generated/scratch"};
        assert(project_source_preparation_capture(&owner, &inputs) == PROJECT_SOURCE_PREPARATION_OK);
        assert(owner.original.valid && owner.configured.view.count == 1 && owner.original_capture->view.count == 2
            && owner.original_capture == &owner.augmentation.capture && owner.mains.capture == owner.original_capture
            && owner.placement.selection == owner.selected && owner.selected->capture == owner.original_capture);
        assert(owner.root_settings.final_count == (move ? 2 : 3) && !owner.complete.attempted && !owner.final.attempted);
        assert(access("generated/copied", F_OK) != 0 && errno == ENOENT);
        assert(source_workspace_cleanup(&owner.workspace) == SOURCE_WORKSPACE_OK && project_source_preparation_destroy(&owner) == 0);
        project_source_request_destroy(&request);
        assert(unlink("generated/source/first") == 0 && unlink("generated/additional/second") == 0
            && rmdir("generated/source") == 0 && rmdir("generated/additional") == 0 && rmdir("generated/scratch") == 0); destroy_test();
    }
}
