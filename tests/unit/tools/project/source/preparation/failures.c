/** @file
 * @brief Exhaust actual original command preparation allocation prefixes and immutable admission failures.
 * @see [T289](../../../../../../docs/implementation/source-operations.md#t289-entry-review-split-from-t287)
 */
/** Every measured allocation traverses real roots, source requests and actual
 * compiler protocol admission. Cleanup retires only owned private material and
 * leaves original inputs and every failed primary child untouched. */
static void it_retires_every_original_command_capture_allocation_failure_and_preserves_single_attempts(void) {
    setup_test(); assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/scratch", 0700) == 0);
    file("generated/source/main", "@!\n");
    const char *arguments[] = {"ama-project", "copy", "generated/source", "generated/copied", "--preview",
        "--standard-root", "generated/source", "--compiler", "../../../../../../ama"};
    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
    assert(project_source_request_parse(&request, (int)(sizeof(arguments)/sizeof(*arguments)), arguments) == PROJECT_SOURCE_REQUEST_OK);
    ProjectSourcePreparationInputs inputs = {.request = &request, .temporary_parent = "generated/scratch"};
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        ProjectSourcePreparation owner; project_source_preparation_init(&owner, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        ProjectSourcePreparationStatus status = project_source_preparation_capture(&owner, &inputs);
        failure_at = 0;
        if (!failure) { assert(status == PROJECT_SOURCE_PREPARATION_OK && owner.captured); total = allocation_count - before; assert(total); }
        else { assert(status != PROJECT_SOURCE_PREPARATION_OK && !owner.captured); }
        size_t after = allocation_count;
        assert(project_source_preparation_capture(&owner, &inputs) == PROJECT_SOURCE_PREPARATION_INVALID
            && owner.status == status && allocation_count == after && !owner.prepared && !owner.complete.attempted);
        assert(source_workspace_cleanup(&owner.workspace) == SOURCE_WORKSPACE_OK);
        assert(owner.status == status && project_source_preparation_destroy(&owner) == 0 && live_allocations == baseline);
        assert(access("generated/copied", F_OK) != 0 && errno == ENOENT);
    }
    ProjectSourcePreparation owner; project_source_preparation_init(&owner, sources.allocator);
    assert(project_source_preparation_capture(NULL, &inputs) == PROJECT_SOURCE_PREPARATION_INVALID);
    assert(project_source_preparation_capture(&owner, NULL) == PROJECT_SOURCE_PREPARATION_INVALID && !owner.captured && !owner.configured.attempted);
    assert(project_source_preparation_destroy(&owner) == 0);
    project_source_request_destroy(&request);
    assert(unlink("generated/source/main") == 0 && rmdir("generated/source") == 0 && rmdir("generated/scratch") == 0); destroy_test();
}
