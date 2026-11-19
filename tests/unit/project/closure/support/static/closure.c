/** @file
 * @brief Compose static expressions with exact roots, source cycles and native dependency ownership.
 */

/** Prepare source-only loading evidence; interface/type legality belongs to
 * later semantic stages and does not decide which written paths are required.
 */
static void prepare_static_closure(void) {
    directory("generated/static project");
    directory("generated/static standard");
    directory("generated/static project/shelf+");
    file("generated/static project/main", "@...\n...input\n...family\n"
        " ...element < .outer\n  ...element < ..item-state\n"
        "...prepared\n.producer\n ...element < .item-state\n"
        ".factory\n ...family > ...alias\n...selected\n...alias\n"
        " ...element < .shelf.child\n");
    file("generated/static project/outer", "@<\nvalue\n...element\n@!\nback&\n.outer\n");
    file("generated/static project/item-state", "@!\n");
    file("generated/static project/producer", "@<\nvalue\n...element\n@!\n");
    file("generated/static project/factory", "@...\n...family\n.producer\n");
    file("generated/static project/shelf", "unfinished [\n");
    file("generated/static project/shelf+/child", "@!\n");
    file("generated/static standard/item-state", "@C\n");
    file("generated/static standard/item-state.h", "/* native header */\n");
    file("generated/static standard/item-state.c", "/* native body */\n");
    file("generated/static standard/item-state.abi", "+ initialize\n- destroy\n");
}

/** Load the same fixed family partition in either configured-root order. */
static ProjectStatus load_static_closure(bool reverse) {
    ModuleRoot roots[] = {{"generated/static project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/static standard", NULL, 0, MODULE_ROOT_STANDARD}};
    if (reverse) { ModuleRoot temporary = roots[0]; roots[0] = roots[1]; roots[1] = temporary; }
    return project_load(&project, roots, 2, ".main");
}

/** Every expression-owning AST position contributes exact source requests;
 * equal names in different families remain distinct and aliases are not paths.
 */
static void it_loads_all_static_expression_positions_without_importing_placeholders_or_parents(void) {
    prepare_static_closure();
    for (size_t order = 0; order < 2; order++) {
        setup_project();
        ProjectStatus status = load_static_closure(order != 0);
        if (status != PROJECT_OK) { fprintf(stderr, "Static closure: %s\n", project.error.message); }
        assert(status == PROJECT_OK && project.valid && module_count() == 7);
        const char *expected[] = {".outer", "..item-state", ".producer", ".item-state", ".factory", ".shelf.child"};
        const ProjectDependency *edge = project.entry->first;
        size_t previous = 0;
        for (size_t index = 0; index < 6; index++) {
            assert(edge != NULL && strcmp(edge->logical_path, expected[index]) == 0);
            assert(edge->target == project_find(&project, expected[index]));
            assert(edge->requirement.source == project.entry->source->text);
            assert(layout_text_is(edge->requirement, expected[index]));
            assert(edge->requirement.start > previous);
            previous = edge->requirement.start;
            edge = edge->next;
        }
        assert(edge == NULL && project_find(&project, ".shelf") == NULL);
        const ProjectModule *native = project_find(&project, "..item-state");
        assert(native != project_find(&project, ".item-state") && native->source->native_loaded);
        assert(native->source->native_header != NULL && native->source->native_c != NULL
            && native->source->native_abi != NULL);
        assert(project_find(&project, ".outer")->first->target == project_find(&project, ".outer"));
        assert(project_find(&project, ".factory")->first->target == project_find(&project, ".producer"));
        for (const ProjectModule *module = project.first; module != NULL; module = module->next) {
            assert(module->state == PROJECT_MODULE_READY && module->syntax.valid);
        }
        destroy_project();
    }
}
