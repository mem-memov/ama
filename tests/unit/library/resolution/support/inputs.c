/** @file
 * @brief Compare exact component dependency snapshots and retained source bytes.
 */
/** Exact source bytes invalidate the component consumer without rewriting earlier snapshots. */
static void it_tracks_dependency_changes_and_preserves_handwritten_inputs(void) {
    prepare_project();
    setup_test();
    LibraryBuild first, same, changed;
    bool valid = resolve(&first, records) && resolve(&same, records);
    assert(valid);
    SourceStatus compared = project_inputs_compare(&same.frontend.inputs, &first.frontend.inputs);
    assert(compared == SOURCE_OK);
    bool dirty = true;
    bool found = project_inputs_changed(&same.frontend.inputs, PROJECT_INPUT_GENERATED, "Ama C library component", &dirty);
    assert(found && !dirty);
    file("generated/project/packages/test/fixture/v1.0.0/src/other", "# changed result initializer\n@>\nmessages[]\n//text\n |new\n@!\n");
    valid = resolve(&changed, records);
    assert(valid);
    compared = project_inputs_compare(&changed.frontend.inputs, &first.frontend.inputs);
    assert(compared == SOURCE_OK);
    found = project_inputs_changed(&changed.frontend.inputs, PROJECT_INPUT_GENERATED, "Ama C library component", &dirty);
    assert(found && dirty);
    assert(strcmp(first.exports->next->type->module->source->logical_path, "/test/fixture/v1.0.0/src/other") == 0);
    const unsigned char *old = source_bytes(first.exports->next->type->module->source->text);
    assert(memcmp(old, "@>", 2) == 0);
    library_build_destroy(&changed);
    library_build_destroy(&same);
    library_build_destroy(&first);
    destroy_test();
}
/** Explicit link files participate in invalidation; native authoring inputs remain unchanged. */
static void it_tracks_link_inputs_and_preserves_native_companions(void) {
    prepare_project();
    const char *body = "#error Library checking must never compile or rewrite native input\n";
    file("generated/project/packages/test/fixture/v1.0.0/src/probe", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/probe.h", body);
    file("generated/project/packages/test/fixture/v1.0.0/src/probe.c", body);
    file("generated/project/packages/test/fixture/v1.0.0/src/probe.abi", "+ initialize\n- destroy\n");
    file("generated/link.input", "original foreign dependency\n");
    const char *tail = "export | probe | /test/fixture/v1.0.0/src/probe | entry,read\nlink-file | generated/link.input\ndependency-file | generated/project/packages/test/fixture/v1.0.0/src/probe.c\n";
    setup_test();
    LibraryBuild before, after;
    bool valid = resolve(&before, tail);
    assert(valid);
    file("generated/link.input", "changed foreign dependency\n");
    valid = resolve(&after, tail);
    assert(valid);
    SourceStatus compared = project_inputs_compare(&after.frontend.inputs, &before.frontend.inputs);
    assert(compared == SOURCE_OK);
    bool dirty = false;
    bool found = project_inputs_changed(&after.frontend.inputs, PROJECT_INPUT_GENERATED, "Ama C library component", &dirty);
    assert(found && dirty);
    FILE *stream = fopen("generated/project/packages/test/fixture/v1.0.0/src/probe.c", "rb");
    assert(stream != NULL);
    char actual[256];
    size_t length = fread(actual, 1, sizeof(actual) - 1, stream);
    int closed = fclose(stream);
    assert(closed == 0 && length == strlen(body) && memcmp(actual, body, length) == 0);
    library_build_destroy(&after);
    library_build_destroy(&before);
    destroy_test();
    setup_test();
    valid = resolve(&after, "export | probe | /test/fixture/v1.0.0/src/probe | entry\nlink-file | generated/absent-input\n");
    assert(!valid && after.error.code == DIAGNOSTIC_IO_FAILURE && !after.valid);
    library_build_destroy(&after);
    destroy_test();
}
