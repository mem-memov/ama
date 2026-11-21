/** @file
 * @brief Verify actual activation and exact invalidation of local alias companions.
 * The input leaf owns the project/source graphs and counted failure fixture.
 */

/** Local creation, empty presence, exact bytes and removal invalidate only real consumers. */
static void it_tracks_local_configuration_presence_and_bytes_without_rereading_disk(void) {
    prepare_inputs();
    setup_project();
    assert(load_root("generated/project") == PROJECT_OK);
    ModuleRoot root = {.path = "generated/project", .kind = MODULE_ROOT_PROJECT};
    const char *path = "generated/project/packages/test/fixture/v1.0.0/src/shared.ini";
    file(path, "");
    ProjectInputs before;
    capture(&before, &project, &root);
    assert(project_inputs_find(&before, PROJECT_INPUT_CONFIGURATION, path) != NULL);
    const char *payloads[] = {"", "~unused = ./unused\n", NULL};
    for (size_t mutation = 0; mutation < 3; mutation++) {
        if (payloads[mutation] == NULL) { assert(remove(path) == 0); }
        else { file(path, payloads[mutation]); }
        Project next;
        project_init(&next, sources.allocator);
        assert(project_load(&next, &root, 1, "/test/fixture/v1.0.0/src/main") == PROJECT_OK);
        ProjectInputs after;
        capture(&after, &next, &root);
        assert(project_inputs_compare(&after, &before) == SOURCE_OK);
        bool expected = mutation < 2;
        assert(changed(&after, PROJECT_INPUT_CONFIGURATION, path) == expected);
        assert(changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/shared") == expected);
        assert(changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/left") == expected);
        assert(changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/main") == expected);
        assert(!changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/right"));
        assert(!changed(&after, PROJECT_INPUT_FILE,
            "generated/project/packages/test/fixture/v1.0.0/src/shared"));
        project_inputs_destroy(&after);
        project_destroy(&next);
    }
    project_inputs_destroy(&before);
    destroy_project();
}

/** Invalid unused companions do not activate; actual use retains original requester diagnostics. */
static void it_reads_local_configuration_only_for_required_modules_with_owned_errors(void) {
    prepare_inputs();
    const char *path = "generated/project/packages/test/fixture/v1.0.0/src/unused.ini";
    file(path, ";\n\xff");
    setup_project();
    assert(load_root("generated/project") == PROJECT_OK);
    const ModuleSource *unused = NULL;
    assert(module_catalog_find(&project.catalog, "/test/fixture/v1.0.0/src/unused", &unused) == MODULE_OK);
    assert(!unused->configuration_loaded && unused->configuration == NULL);
    Project next;
    project_init(&next, sources.allocator);
    ModuleRoot root = {.path = "generated/project", .kind = MODULE_ROOT_PROJECT};
    file("generated/project/packages/test/fixture/v1.0.0/src/main",
        "@!\nvalue\n/test/fixture/v1.0.0/src/unused\n");
    assert(project_load(&next, &root, 1, "/test/fixture/v1.0.0/src/main") == PROJECT_CATALOG_ERROR);
    assert(!next.valid && strcmp(source_path(next.error.primary.span.source), path) == 0);
    assert(next.error.primary.span.start == 2 && next.error.related_count == 1);
    assert(next.error.related[0].location.span.source == next.entry->source->text);
    assert(diagnostic_valid(&next.error));
    project_destroy(&next);
    destroy_project();
}
