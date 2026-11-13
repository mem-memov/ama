/** @file
 * @brief Fail every semantic allocation in combined branch, header chain, traversal binding, local transfer and lexical-break resolution.
 */
/** Complete typed control metadata is published only after all measured construction and deferred-resolution allocations succeed. */
static void it_unwinds_every_allocation_in_complete_control_contexts(void) {
    prepare_control();
    const char *text = "@!\nouter\n.value\nroot\n.producer\nitems[]\n.value\nouter\n{\n root !\n  < |bytes\n"
        "  child !\n   condition\n   {\n    items[item&]\n    {\n     saved&\n      item&\n     ^\n    }\n   }\n :\n  ^\n}\n";
    SemanticStatus status = resolve_control(text);
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
        status = semantic_names(&semantic);
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
