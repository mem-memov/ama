/** @file
 * @brief Verify unresolved graph kinds and export selectors cannot leak into concrete runtime types.
 */

/** Syntax admission is not specialization admission: even an empty graph must
 * be refused by the current concrete inventory, as must selected field and
 * namespace Types awaiting the explicit T212/T184 bridge.
 */
static void it_refuses_static_graphs_and_selected_types_before_concrete_inventory_publication(void) {
    const char *cases[] = {"@...\n", "\n# static graph\n\n@... # marker\n",
        "@!\nfield\n.other\n ...ready\n",
        "@!\n.other\n ...ready\n zero >> value\n"};
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        directory("generated/project");
        file("generated/project/main", cases[index]);
        file("generated/project/other", "@...\n...ready\n.main\n");
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        assert(!compiler_frontend_check(&frontend, &root, 1, ".main"));
        assert(!frontend.valid && frontend.static_inventory.valid && frontend.semantic_initialized);
        assert(!frontend.semantic.names_valid && !frontend.plan_initialized);
        assert(diagnostic_valid(&frontend.error));
        assert(strstr(frontend.error.message, "concrete declaration inventory") != NULL);
        assert(frontend.error.primary.span.source == frontend.project.entry->source->text);
        if (frontend.project.entry->syntax.sections.kind == MODULE_STATIC) {
            SourceSpan marker = frontend.project.entry->syntax.sections.graph.marker->code;
            assert(frontend.error.primary.span.start == marker.start);
            assert(frontend.error.primary.span.end == marker.end);
            assert(layout_text_is(frontend.error.primary.span, "@..."));
        }
        destroy_frontend();
    }
}
