/** @file
 * @brief Independently compile module-alias recipe output under its actual copied namespace and package scope.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Both kinds preserve keys/comments and original alias bindings. Ordinary source
 * occurrences are rooted separately so moving the local base changes no edge. */
static void it_prepares_owned_module_alias_writes_and_compiles_actual_copied_companions(void) {
    for (unsigned standard = 0; standard < 2; standard++) {
        ModuleWriteFixture fixture; module_write_fixture(&fixture, standard != 0);
        SourceOperationModuleWrite write; source_operation_module_write_init(&write, sources.allocator);
        assert(source_operation_module_write_prepare(&write, &fixture.inputs) == SOURCE_OPERATION_MODULE_OK && write.valid);
        const char *expected = standard ? "; local aliases\n~local = //missing\n"
            : "; local aliases\n~local = /vendor/pkg/v1.0.0/src/other\n";
        assert(source_length(write.result) == strlen(expected) && !memcmp(source_bytes(write.result), expected, strlen(expected)));
        SourceGraphPreparedText main = {.edits = &fixture.main_edits};
        const SourceOperationModuleWrite *module_writes[] = {&write};
        SourceOperationWritesInputs joined_inputs = {.copy = &copy, .observed = &fixture.observed,
            .mains = &fixture.mains, .files = &fixture.files, .main_texts = &main, .modules = module_writes, .module_count = 1};
        SourceOperationWrites joined; source_operation_writes_init(&joined, sources.allocator);
        assert(source_operation_writes_build(&joined, &joined_inputs) == SOURCE_OPERATION_WRITES_OK && joined.valid && !joined.count);
        assert(joined.first && joined.first->kind == SOURCE_OPERATION_PREPARED_MAIN && joined.first->next
            && joined.first->next->kind == SOURCE_OPERATION_PREPARED_MODULE && !joined.first->next->next);
        check_source_write_join_failures(&joined_inputs);
        const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
        assert(source_copy_image_prepare(&fixture.arena, &fixture.observed.image, &fixture.files, joined.prepared, joined.changes, joined.count,
            capture.roots, capture.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
        SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
        assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
        FILE *request = fopen("generated/module-configuration-write.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
        SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
        assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/module-configuration-write.json", &final.view, NULL) == SOURCE_COMPILER_OK);
        SourceCopyProofIssue issue;
        assert(source_copy_prove(&fixture.arena, &analysis, &fixture.observed, &fixture.mains, &fixture.files, joined.prepared,
            &fixture.contexts, &final, &client, NULL, 0, &issue) == SOURCE_COPY_PROOF_OK);
        check_complete_retained_roots(&joined_inputs, &fixture.contexts, &client, SOURCE_OPERATION_SCOPE_PRESERVED);
        size_t before = allocation_count;
        assert(source_operation_module_write_prepare(&write, &fixture.inputs) == SOURCE_OPERATION_MODULE_INVALID && write.valid && allocation_count == before);
        source_compiler_client_destroy(&client); source_image_capture_destroy(&final);
        source_operation_writes_destroy(&joined); source_operation_module_write_destroy(&write); module_write_fixture_destroy(&fixture, standard != 0);
    }
}
