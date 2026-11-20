/** @file
 * @brief Fail every semantic allocation while specializing all fixed list interfaces and linking ephemeral output actions.
 */
/** Real two-root source/native setup precedes each independent semantic allocation failure, including profile diagnostics and action publication. */
static void it_unwinds_every_list_interface_binding_and_result_allocation(void) {
    prepare_lists();
    const char *text = "@!\nitems[]\n.value\nsample\n.value\nitems[]\n append !\n  < &sample\n  > element&\n"
        " insert !\n  < |0\n  <\n length !\n  > count\n  value !\n clear !\n";
    SemanticStatus status = resolve_list(text);
    assert(status == SEMANTIC_OK);
    size_t count = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        file("generated/program/main", text);
        setup_semantic();
        ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
        ProjectStatus loaded = project_load(&project, roots, 2, ".main");
        assert(loaded == PROJECT_OK);
        semantic_program_init(&semantic, &project, sources.allocator);
        status = semantic_fixture_names(&semantic);
        assert(status == SEMANTIC_OK);
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OK);
        if (failure <= allocation_count) { destroy_semantic(); continue; }
        failure_at = failure;
        status = semantic_resolve(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.resolved && diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
