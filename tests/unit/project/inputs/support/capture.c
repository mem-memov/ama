/**
 * @file
 * @brief Compose real immutable source closures, native changes, root settings and failure ownership.
 */

/** Create one diamond-free source graph with an unrelated right branch and a native shared dependency. */
static void prepare_inputs(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nleft\n/test/fixture/v1.0.0/src/left\nright\n/test/fixture/v1.0.0/src/right\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/left", "@!\nshared\n/test/fixture/v1.0.0/src/shared\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/right", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/shared", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/shared.c", "/* native body */\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/shared.h", "/* native header */\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/shared.abi", "+ initialize\n- destroy\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/unused", "@!\n");
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
    const char *paths[] = {"generated/project/packages/test/fixture/v1.0.0/src/shared.c", "generated/project/packages/test/fixture/v1.0.0/src/shared.h",
        "generated/project/packages/test/fixture/v1.0.0/src/shared.abi", "generated/project/packages/test/fixture/v1.0.0/src/shared", "generated/project/packages/test/fixture/v1.0.0/src/unused"};
    const char *contents[] = {"/* changed native body */\n", "/* changed native header */\n",
        "+ another_constructor\n- destroy\n", "@C\n# changed\n", "@!\n# changed unused\n"};
    for (size_t mutation = 0; mutation < 5; mutation++) {
        file(paths[mutation], contents[mutation]);
        Project after_project;
        project_init(&after_project, sources.allocator);
        loaded = project_load(&after_project, &root, 1, "/test/fixture/v1.0.0/src/main");
        assert(loaded == PROJECT_OK);
        ProjectInputs after;
        capture(&after, &after_project, &root);
        SourceStatus status = project_inputs_compare(&after, &before);
        assert(status == SOURCE_OK);
        assert(changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/shared") == (mutation < 4));
        assert(changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/left") == (mutation < 4));
        assert(changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/main") == (mutation < 4));
        assert(!changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/right"));
        assert(project_inputs_find(&after, PROJECT_INPUT_FILE, "generated/project/packages/test/fixture/v1.0.0/src/unused") == NULL);
        project_inputs_destroy(&after);
        project_destroy(&after_project);
        prepare_inputs();
    }
    const char *excluded[] = {"owned output"};
    root.excluded = excluded;
    root.excluded_count = 1;
    Project configured_project;
    project_init(&configured_project, sources.allocator);
    loaded = project_load(&configured_project, &root, 1, "/test/fixture/v1.0.0/src/main");
    assert(loaded == PROJECT_OK);
    ProjectInputs configured;
    capture(&configured, &configured_project, &root);
    SourceStatus status = project_inputs_compare(&configured, &before);
    assert(status == SOURCE_OK);
    const char *names[] = {"/test/fixture/v1.0.0/src/main", "/test/fixture/v1.0.0/src/left", "/test/fixture/v1.0.0/src/right", "/test/fixture/v1.0.0/src/shared"};
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
    int removed = remove("generated/project/packages/test/fixture/v1.0.0/src/shared.c");
    assert(removed == 0);
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    ProjectInputs inputs;
    project_inputs_init(&inputs, sources.allocator);
    SourceStatus status = project_inputs_capture(&inputs, &project, &root, 1);
    assert(status == SOURCE_IO_ERROR && !inputs.sealed);
    project_destroy(&project);
    assert(strcmp(inputs.failed_path, "generated/project/packages/test/fixture/v1.0.0/src/shared.c") == 0);
    project_inputs_destroy(&inputs);
    destroy_test();
}

/** Captured package bytes survive disk changes; only the new project's consumers invalidate. */
static void it_tracks_retained_package_configuration_without_rereading_disk(void) {
    prepare_inputs();
    setup_project();
    assert(load_root("generated/project") == PROJECT_OK);
    ModuleRoot root = {.path = "generated/project", .kind = MODULE_ROOT_PROJECT};
    const char *path = "generated/project/packages/test/fixture/v1.0.0/package.ini";
    file(path, "; changed after catalog acquisition\n");
    ProjectInputs before;
    capture(&before, &project, &root);
    assert(project_inputs_find(&before, PROJECT_INPUT_CONFIGURATION, path) != NULL);
    assert(project_inputs_find(&before, PROJECT_INPUT_FILE, path) == NULL);
    Project after_project;
    project_init(&after_project, sources.allocator);
    assert(project_load(&after_project, &root, 1, "/test/fixture/v1.0.0/src/main") == PROJECT_OK);
    ProjectInputs after;
    capture(&after, &after_project, &root);
    assert(project_inputs_compare(&after, &before) == SOURCE_OK);
    assert(changed(&after, PROJECT_INPUT_CONFIGURATION, path));
    const char *names[] = {"main", "left", "right", "shared"};
    for (size_t index = 0; index < 4; index++) {
        char name[128];
        int length = snprintf(name, sizeof(name), "/test/fixture/v1.0.0/src/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(name));
        assert(changed(&after, PROJECT_INPUT_MODULE, name));
        assert(!changed(&after, PROJECT_INPUT_FILE,
            source_path(project_find(&after_project, name)->source->text)));
    }
    project_inputs_destroy(&after);
    project_destroy(&after_project);
    project_inputs_destroy(&before);
    destroy_project();
}
