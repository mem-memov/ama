/**
 * @file
 * @brief Compose real immutable source closures, native changes, root settings and failure ownership.
 */

/** Create one diamond-free source graph with an unrelated right branch and a native shared dependency. */
static void prepare_inputs(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nleft\n.left\nright\n.right\n");
    file("generated/project/left", "@!\nshared\n.shared\n");
    file("generated/project/right", "@C\n");
    file("generated/project/shared", "@C\n");
    file("generated/project/shared.c", "/* native body */\n");
    file("generated/project/shared.h", "/* native header */\n");
    file("generated/project/shared.abi", "+ initialize\n- destroy\n");
    file("generated/project/unused", "@!\n");
}
/** Capture a complete project with a stable explicit root configuration. */
static void capture(ProjectInputs *inputs, Project *owner, const ModuleRoot *root) {
    project_inputs_init(inputs, sources.allocator);
    SourceStatus status = project_inputs_capture(inputs, owner, root, 1);
    assert(status == SOURCE_OK);
    project_inputs_seal(inputs);
}
/** C/header/ABI/source changes affect only the owning module and its transitive consumers; root changes affect every resolver consumer. */
static void it_invalidates_exact_native_consumers_and_complete_root_configuration(void) {
    prepare_inputs();
    setup_project();
    ProjectStatus loaded = load_root("generated/project");
    assert(loaded == PROJECT_OK && module_count() == 4);
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    ProjectInputs before;
    capture(&before, &project, &root);
    const char *paths[] = {"generated/project/shared.c", "generated/project/shared.h",
        "generated/project/shared.abi", "generated/project/shared", "generated/project/unused"};
    const char *contents[] = {"/* changed native body */\n", "/* changed native header */\n",
        "+ another_constructor\n- destroy\n", "@C\n# changed\n", "@!\n# changed unused\n"};
    for (size_t mutation = 0; mutation < 5; mutation++) {
        file(paths[mutation], contents[mutation]);
        Project after_project;
        project_init(&after_project, sources.allocator);
        loaded = project_load(&after_project, &root, 1, ".main");
        assert(loaded == PROJECT_OK);
        ProjectInputs after;
        capture(&after, &after_project, &root);
        SourceStatus status = project_inputs_compare(&after, &before);
        assert(status == SOURCE_OK);
        assert(changed(&after, PROJECT_INPUT_MODULE, ".shared") == (mutation < 4));
        assert(changed(&after, PROJECT_INPUT_MODULE, ".left") == (mutation < 4));
        assert(changed(&after, PROJECT_INPUT_MODULE, ".main") == (mutation < 4));
        assert(!changed(&after, PROJECT_INPUT_MODULE, ".right"));
        assert(project_inputs_find(&after, PROJECT_INPUT_FILE, "generated/project/unused") == NULL);
        project_inputs_destroy(&after);
        project_destroy(&after_project);
        prepare_inputs();
    }
    const char *excluded[] = {"owned output"};
    root.excluded = excluded;
    root.excluded_count = 1;
    Project configured_project;
    project_init(&configured_project, sources.allocator);
    loaded = project_load(&configured_project, &root, 1, ".main");
    assert(loaded == PROJECT_OK);
    ProjectInputs configured;
    capture(&configured, &configured_project, &root);
    SourceStatus status = project_inputs_compare(&configured, &before);
    assert(status == SOURCE_OK);
    const char *names[] = {".main", ".left", ".right", ".shared"};
    for (size_t index = 0; index < 4; index++) { assert(changed(&configured, PROJECT_INPUT_MODULE, names[index])); }
    project_inputs_destroy(&configured);
    project_destroy(&configured_project);
    project_inputs_destroy(&before);
    destroy_project();
}
/** Fail every real capture allocation after project acquisition; every partial graph remains unsealed and fully destructible. */
static void it_unwinds_every_capture_allocation_failure_without_sealing_partial_graphs(void) {
    prepare_inputs();
    setup_project();
    ProjectStatus loaded = load_root("generated/project");
    assert(loaded == PROJECT_OK);
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    size_t start = allocation_count;
    ProjectInputs inputs;
    capture(&inputs, &project, &root);
    size_t count = allocation_count - start;
    project_inputs_destroy(&inputs);
    for (size_t failure = 1; failure <= count; failure++) {
        project_inputs_init(&inputs, sources.allocator);
        failure_at = allocation_count + failure;
        SourceStatus status = project_inputs_capture(&inputs, &project, &root, 1);
        assert(status == SOURCE_OUT_OF_MEMORY && !inputs.sealed);
        status = project_inputs_compare(&inputs, NULL);
        assert(status == SOURCE_INVALID_ARGUMENT);
        project_inputs_destroy(&inputs);
        failure_at = 0;
    }
    destroy_project();
}
/** A companion disappearing between catalog acquisition and capture produces an owned exact IO failure path. */
static void it_owns_the_failed_native_input_path_after_project_destruction(void) {
    prepare_inputs();
    setup_project();
    ProjectStatus loaded = load_root("generated/project");
    assert(loaded == PROJECT_OK);
    int removed = remove("generated/project/shared.c");
    assert(removed == 0);
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    ProjectInputs inputs;
    project_inputs_init(&inputs, sources.allocator);
    SourceStatus status = project_inputs_capture(&inputs, &project, &root, 1);
    assert(status == SOURCE_IO_ERROR && !inputs.sealed);
    project_destroy(&project);
    assert(strcmp(inputs.failed_path, "generated/project/shared.c") == 0);
    project_inputs_destroy(&inputs);
    destroy_test();
}
