/** @file
 * @brief Verify that actual explicit data-main ownership reaches copy preparation independently of mode.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** A declared main brings its full source/test facts even in the default external-dependency mode. */
static void it_includes_declared_module_data_in_default_copy_mode_with_its_real_tests(void) {
    prepare_profile(true, false, false); select_main();
    const SourceCapturedUnit *main = capture.units, *other = capture.units;
    while (main && strcmp(main->source->logical_path, main_identity)) { main = main->next; }
    while (other && strcmp(other->source->logical_path, "/vendor/pkg/v1.0.0/src/other")) { other = other->next; }
    assert(main && other);
    SourceDeclaredGroup declared = {.requester = main, .version = other->source->version, .files = other->files};
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, &declared);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
    const AcquisitionRoleRecord *selected = copy_role(other->source->logical_path); assert(selected && selected->expanded);
    assert(selected->reasons && selected->reasons->kind == ACQUISITION_DECLARED_MODULE && !selected->reasons->next);
    assert(copy_role("/vendor/pkg/v1.0.0/tests/other/check") && !copy_role("//plain")); finish_copy();
}
/** Changed original observations refuse rather than recapturing a new program or publishing files. */
static void it_refuses_changed_original_unit_bytes_and_new_companions_before_copy_acceptance(void) {
    for (unsigned companion = 0; companion < 2; companion++) {
        prepare(); select_main();
        const char *path = companion ? source_arena_join(&tree.arena, tree.operand.path, "", ".new") : tree.operand.path; assert(path);
        file(path, companion ? "new opaque companion\n" : "@!\nchanged\n");
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
        assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_EXPANSION);
        assert(copy.expansion_status == ACQUISITION_CLOSURE_CONFLICT && !copy.valid && !copy.closure.valid && copy.failed_path);
        assert(access(path, F_OK) == 0 && !analysis.probes);
        if (companion) { assert(unlink(path) == 0); }
        finish_copy();
    }
}
