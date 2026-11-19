/** @file
 * @brief Verify transitive invalidation through real static graph and selector edges.
 */

/** The static namespace path and its nested concrete input both participate in
 * the ordinary consumer's input closure; unrelated source files stay absent.
 */
static void it_invalidates_static_producers_and_selected_consumers_from_nested_sources(void) {
    directory("generated/static inputs");
    file("generated/static inputs/main", "@!\nvalue\n.assembly\n ...export\n");
    file("generated/static inputs/assembly", "@...\n...export\n.box\n ...element < .element\n");
    file("generated/static inputs/box", "@<\nvalue\n...element\n@!\n");
    file("generated/static inputs/element", "@!\n");
    file("generated/static inputs/unused", "@!\n");
    setup_project();
    ModuleRoot root = {"generated/static inputs", NULL, 0, MODULE_ROOT_PROJECT};
    assert(project_load(&project, &root, 1, ".main") == PROJECT_OK);
    ProjectInputs before;
    capture(&before, &project, &root);
    file("generated/static inputs/element", "@!\n# changed source\n");
    Project replacement;
    project_init(&replacement, sources.allocator);
    assert(project_load(&replacement, &root, 1, ".main") == PROJECT_OK);
    ProjectInputs after;
    capture(&after, &replacement, &root);
    assert(project_inputs_compare(&after, &before) == SOURCE_OK);
    assert(changed(&after, PROJECT_INPUT_MODULE, ".element"));
    assert(changed(&after, PROJECT_INPUT_MODULE, ".assembly"));
    assert(changed(&after, PROJECT_INPUT_MODULE, ".main"));
    assert(!changed(&after, PROJECT_INPUT_MODULE, ".box"));
    assert(project_inputs_find(&after, PROJECT_INPUT_MODULE, ".unused") == NULL);
    project_inputs_destroy(&after);
    project_destroy(&replacement);
    project_inputs_destroy(&before);
    destroy_project();
}
