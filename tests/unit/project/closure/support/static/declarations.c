/** @file
 * @brief Verify exact dependency roles and spans in nested declaration Type trees.
 */

/** Loading follows every authored head in lexical order without interpreting
 * a foreign binding label or selected export as another module dependency.
 */
static void it_discovers_nested_declaration_inputs_and_selectors_without_losing_source_roles(void) {
    const char *root = "generated/declaration paths";
    fixture_package_prepare(root, "fixture");
    file("generated/declaration paths/packages/test/fixture/v1.0.0/src/main",
        "@!\nvalue\n./outer\n .first < ./leaf\n .second < ./factory\n"
        "  .slot < ./inner\n   .element < ./leaf\n  .result\n");
    file("generated/declaration paths/packages/test/fixture/v1.0.0/src/outer",
        "@<\na\n.first\nb\n.second\n@!\n");
    file("generated/declaration paths/packages/test/fixture/v1.0.0/src/inner",
        "@<\nitem\n.element\n@!\n");
    file("generated/declaration paths/packages/test/fixture/v1.0.0/src/factory",
        "@...\n.result\n.slot\n");
    file("generated/declaration paths/packages/test/fixture/v1.0.0/src/leaf", "@!\n");
    setup_project();
    assert(load_root(root) == PROJECT_OK && project.valid && module_count() == 5);
    const char *heads[] = {"./outer", "./leaf", "./factory", "./inner", "./leaf"};
    const ProjectDependency *edge = project.entry->first;
    const ProjectModule *leaf = NULL;
    size_t previous = 0;
    for (size_t index = 0; index < sizeof(heads) / sizeof(*heads); index++) {
        assert(edge && !edge->implicit && edge->requirement.source == project.entry->source->text);
        assert(layout_text_is(edge->requirement, heads[index]) && edge->requirement.start > previous);
        assert(edge->role == (index == 2 ? PROJECT_STATIC_SELECTOR : PROJECT_ORDINARY_PATH));
        assert(strcmp(edge->logical_path, edge->target->source->logical_path) == 0);
        if (index == 1) { leaf = edge->target; }
        if (index == 4) { assert(edge->target == leaf); }
        previous = edge->requirement.start;
        edge = edge->next;
    }
    assert(edge == NULL);
    destroy_project();
}
