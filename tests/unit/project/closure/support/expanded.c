/** @file
 * @brief Cover dependency paths in every selected-operation, folded-header and traversal subtree.
 */
/** Prepare declarations and qualified sources whose exact lexical order crosses every AST ownership boundary. */
static void prepare_expanded_closure(void) {
    fixture_package_prepare("generated/expanded", "fixture");
    file("generated/expanded/packages/test/fixture/v1.0.0/src/main",
        "@!\nroot !\n < /test/fixture/v1.0.0/src/rootinput/value\n first !\n  < /test/fixture/v1.0.0/src/firstinput/value\n  child !\n   < /test/fixture/v1.0.0/src/nestedinput/value\n  :\n   field\n   /test/fixture/v1.0.0/src/childhandler\n :\n  field\n  /test/fixture/v1.0.0/src/firsthandler\n > ordinary\n second %\n  > receiver\n :\n  field\n  /test/fixture/v1.0.0/src/secondhandler\n:\n field\n /test/fixture/v1.0.0/src/roothandler\n"
        "header !\n < /test/fixture/v1.0.0/src/headerinput/value\n method !\n  < /test/fixture/v1.0.0/src/innerinput/value\n  condition\n  {\n   body field\n   /test/fixture/v1.0.0/src/loopbody\n   items[item&]\n   {\n    field\n    /test/fixture/v1.0.0/src/eachbody\n   }\n  }\n :\n  field\n  /test/fixture/v1.0.0/src/nestedhandler\n:\n field\n /test/fixture/v1.0.0/src/loophandler\n");
    const char *names[] = {"rootinput", "firstinput", "nestedinput", "childhandler", "firsthandler",
        "secondhandler", "roothandler", "headerinput", "innerinput", "loopbody", "eachbody", "nestedhandler", "loophandler"};
    for (size_t index = 0; index < sizeof(names) / sizeof(names[0]); index++) {
        char path[256];
        int length = snprintf(path, sizeof(path), "generated/expanded/packages/test/fixture/v1.0.0/src/%s", names[index]);
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
    const char *expected[] = {"/test/fixture/v1.0.0/src/rootinput", "/test/fixture/v1.0.0/src/firstinput", "/test/fixture/v1.0.0/src/nestedinput", "/test/fixture/v1.0.0/src/childhandler", "/test/fixture/v1.0.0/src/firsthandler",
        "/test/fixture/v1.0.0/src/secondhandler", "/test/fixture/v1.0.0/src/roothandler", "/test/fixture/v1.0.0/src/headerinput", "/test/fixture/v1.0.0/src/innerinput", "/test/fixture/v1.0.0/src/loopbody", "/test/fixture/v1.0.0/src/eachbody", "/test/fixture/v1.0.0/src/nestedhandler", "/test/fixture/v1.0.0/src/loophandler"};
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
    assert(project_find(&project, "/test/fixture/v1.0.0/src/first") == NULL && project_find(&project, "/test/fixture/v1.0.0/src/condition") == NULL);
    destroy_project();
}
/** Missing transitive and required-draft paths preserve source requester chains originating inside expanded syntax. */
static void it_reports_requester_chains_from_expanded_header_and_body_paths(void) {
    prepare_expanded_closure();
    file("generated/expanded/packages/test/fixture/v1.0.0/src/loopbody", "@!\nfield\n/test/fixture/v1.0.0/src/missing\n");
    setup_project();
    ProjectStatus status = load_root("generated/expanded");
    assert(status == PROJECT_MISSING && project.error.related_count == 2);
    assert(strcmp(project.error.related[0].name, "/test/fixture/v1.0.0/src/loopbody") == 0);
    assert(strcmp(project.error.related[1].name, "/test/fixture/v1.0.0/src/main") == 0);
    assert(layout_text_is(project.error.related[1].location.span, "/test/fixture/v1.0.0/src/loopbody"));
    destroy_project();
    prepare_expanded_closure();
    file("generated/expanded/packages/test/fixture/v1.0.0/src/firstinput", "unfinished[\n");
    setup_project();
    status = load_root("generated/expanded");
    assert(status == PROJECT_DRAFT && layout_text_is(project.error.primary.span, "/test/fixture/v1.0.0/src/firstinput"));
    assert(project.error.related_count == 1 && strcmp(project.error.related[0].name, "/test/fixture/v1.0.0/src/main") == 0);
    destroy_project();
}
