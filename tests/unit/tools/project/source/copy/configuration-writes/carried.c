/** @file
 * @brief Consume authorized carried configuration output through actual shared image and compiler proofs.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Actual raw package.ini placement supplies each version-pair endpoint. Include
 * alias/comment bytes so all copied alias allocations and their failed prefixes
 * are exercised rather than authorizing a table from a matching basename. */
static void check_carried_configuration_write(const SourceOperationPlan *accepted, const SourceOperationIntent *intent) {
    SourcePackageEnvironment original; source_package_environment_init(&original, sources.allocator);
    assert(source_package_environment_build(&original, &accepted->inputs.observed->original->catalog) == SOURCE_ENVIRONMENT_OK);
    assert(original.first && !original.first->next && accepted->environment.first && !accepted->environment.first->next);
    const DependencyViewEntry *before = NULL, *after = NULL;
    for (size_t i = 0; i < original.view->count; i++) {
        if (original.view->entries[i].shape.extent == MODULE_PATH_VERSION) { before = &original.view->entries[i]; }
    }
    for (size_t i = 0; i < accepted->environment.view->count; i++) {
        if (accepted->environment.view->entries[i].shape.extent == MODULE_PATH_VERSION) { after = &accepted->environment.view->entries[i]; }
    }
    assert(before && after); SourceSelectionVersionPair pair = {before, after};
    SourceOperationConfigurationWriteInputs inputs = {.kind = SOURCE_OPERATION_CONFIGURATION_CARRIED,
        .copy = accepted->inputs.copy, .mains = accepted->inputs.mains, .files = accepted->inputs.files,
        .observed = accepted->inputs.observed, .original = &original, .before = original.first,
        .preview = &accepted->environment, .after = accepted->environment.first, .versions = &pair, .version_count = 1};
    SourceOperationConfigurationWrite write; source_operation_configuration_write_init(&write, sources.allocator);
    assert(source_operation_configuration_write_prepare(&write, &inputs) == SOURCE_OPERATION_CONFIGURATION_OK && write.valid);
    assert(!write.roots.count);
    if (inputs.before->aliases->count) {
        const char *expected = "; original spelling\n[aliases]\n~helper = /vendor/pkg/v1.0.0/src/helper\n";
        assert(source_length(write.result) == strlen(expected) && !memcmp(source_bytes(write.result), expected, strlen(expected)));
    } else { assert(!source_length(write.result)); }
    const SourceOperationConfigurationWrite *configurations[] = {&write};
    SourceOperationWritesInputs joined_inputs = {.copy = accepted->inputs.copy, .observed = accepted->inputs.observed,
        .mains = accepted->inputs.mains, .files = accepted->inputs.files, .main_texts = accepted->inputs.texts,
        .removal = accepted->inputs.removal, .configurations = configurations, .configuration_count = 1};
    SourceOperationWrites joined; source_operation_writes_init(&joined, sources.allocator);
    assert(source_operation_writes_build(&joined, &joined_inputs) == SOURCE_OPERATION_WRITES_OK && joined.valid);
    check_source_write_join_failures(&joined_inputs);
    SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
    SourceOperationPlanInputs revised = accepted->inputs;
    revised.texts = joined.prepared; revised.changes = joined.changes; revised.change_count = joined.count;
    assert(source_operation_plan_build(&plan, &revised) == SOURCE_OPERATION_PLAN_OK);
    FILE *request = fopen("generated/carried-configuration.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &plan.capture.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/carried-configuration.json", &plan.capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_operation_plan_prove(&plan, &client, &proof) == SOURCE_COPY_PROOF_OK);
    check_complete_operation(&joined_inputs, &plan, intent, &client);
    source_compiler_client_destroy(&client); source_operation_plan_destroy(&plan); source_operation_writes_destroy(&joined); source_operation_configuration_write_destroy(&write);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_operation_configuration_write_init(&write, sources.allocator);
        size_t start = allocation_count; if (failure) { failure_at = start + failure; }
        SourceOperationConfigurationWriteStatus status = source_operation_configuration_write_prepare(&write, &inputs);
        if (!failure) { assert(status == SOURCE_OPERATION_CONFIGURATION_OK); total = allocation_count - start; assert(total); }
        else { assert(status != SOURCE_OPERATION_CONFIGURATION_OK && !write.valid && !write.result); }
        failure_at = 0; source_operation_configuration_write_destroy(&write); assert(live_allocations == baseline);
    }
    SourceSelectionVersionPair foreign = {before, &accepted->environment.view->entries[0]};
    for (size_t i = 0; i < accepted->environment.view->count; i++) {
        if (accepted->environment.view->entries[i].shape.extent == MODULE_PATH_MODULE) { foreign.final = &accepted->environment.view->entries[i]; }
    }
    inputs.versions = &foreign; source_operation_configuration_write_init(&write, sources.allocator);
    assert(source_operation_configuration_write_prepare(&write, &inputs) == SOURCE_OPERATION_CONFIGURATION_SCOPE && !write.valid);
    source_operation_configuration_write_destroy(&write); source_package_environment_destroy(&original);
}
