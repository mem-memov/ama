/** @file
 * @brief Cover dependency paths in every selected-operation, folded-header and traversal subtree.
 */
/** Prepare declarations and qualified sources whose exact lexical order crosses every AST ownership boundary. */
static void prepare_expanded_closure(void) {
    directory("generated/expanded");
    file("generated/expanded/main",
        "@!\nroot !\n < .rootinput.value\n first !\n  < .firstinput.value\n  child !\n   < .nestedinput.value\n  :\n   field\n   .childhandler\n :\n  field\n  .firsthandler\n > ordinary\n second %\n  > receiver\n :\n  field\n  .secondhandler\n:\n field\n .roothandler\n"
        "header !\n < .headerinput.value\n method !\n  < .innerinput.value\n  condition\n  {\n   body field\n   .loopbody\n   items[item&]\n   {\n    field\n    .eachbody\n   }\n  }\n :\n  field\n  .nestedhandler\n:\n field\n .loophandler\n");
    const char *names[] = {"rootinput", "firstinput", "nestedinput", "childhandler", "firsthandler",
        "secondhandler", "roothandler", "headerinput", "innerinput", "loopbody", "eachbody", "nestedhandler", "loophandler"};
    for (size_t index = 0; index < sizeof(names) / sizeof(names[0]); index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/expanded/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, "@!\n");
    }
}
/** Registry admission follows physical positions even when headers/body/handlers occupy different linked AST views. */
static void it_discovers_expanded_trees_once_in_exact_physical_request_order(void) {
    prepare_expanded_closure();
    setup_project();
    ProjectStatus status = load_root("generated/expanded");
    assert(status == PROJECT_OK && module_count() == 14);
    const char *expected[] = {".rootinput", ".firstinput", ".nestedinput", ".childhandler", ".firsthandler",
        ".secondhandler", ".roothandler", ".headerinput", ".innerinput", ".loopbody", ".eachbody", ".nestedhandler", ".loophandler"};
    const ProjectDependency *edge = project.entry->first;
    const ProjectModule *module = project.first->next;
    size_t previous = 0;
    for (size_t index = 0; index < sizeof(expected) / sizeof(expected[0]); index++) {
        assert(edge != NULL && module != NULL);
        assert(strcmp(edge->logical_path, expected[index]) == 0 && edge->target == module);
        assert(module == project_find(&project, expected[index]) && module->requested_by == project.entry);
        assert(edge->requirement.start > previous);
        previous = edge->requirement.start;
        edge = edge->next;
        module = module->next;
    }
    assert(edge == NULL && module == NULL);
    assert(project_find(&project, ".first") == NULL && project_find(&project, ".condition") == NULL);
    destroy_project();
}
/** Missing transitive and required-draft paths preserve source requester chains originating inside expanded syntax. */
static void it_reports_requester_chains_from_expanded_header_and_body_paths(void) {
    prepare_expanded_closure();
    file("generated/expanded/loopbody", "@!\nfield\n.missing\n");
    setup_project();
    ProjectStatus status = load_root("generated/expanded");
    assert(status == PROJECT_MISSING && project.error.related_count == 2);
    assert(strcmp(project.error.related[0].name, ".loopbody") == 0);
    assert(strcmp(project.error.related[1].name, ".main") == 0);
    assert(layout_text_is(project.error.related[1].location.span, ".loopbody"));
    destroy_project();
    prepare_expanded_closure();
    file("generated/expanded/firstinput", "unfinished[\n");
    setup_project();
    status = load_root("generated/expanded");
    assert(status == PROJECT_DRAFT && layout_text_is(project.error.primary.span, ".firstinput"));
    assert(project.error.related_count == 1 && strcmp(project.error.related[0].name, ".main") == 0);
    destroy_project();
}
