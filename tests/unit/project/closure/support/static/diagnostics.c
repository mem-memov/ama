/** @file
 * @brief Check exact nested-static requester diagnostics without cross-family fallback.
 */

/** Write a dependency nested under a placeholder and an identically named
 * project module that must never satisfy the missing Standard request.
 */
static void prepare_missing_static_dependency(void) {
    prepare_static_closure();
    file("generated/static project/packages/test/fixture/v1.0.0/src/main", "@...\n.value\n/test/fixture/v1.0.0/src/outer\n .element < /test/fixture/v1.0.0/src/middle\n");
    file("generated/static project/packages/test/fixture/v1.0.0/src/middle", "@...\n.value\n.family\n .element < //absent\n");
    file("generated/static project/packages/test/fixture/v1.0.0/src/absent", "@!\n");
    int removed = remove("generated/static standard/absent");
    assert(removed == 0 || errno == ENOENT);
}

/** Missing/draft/invalid concrete arguments retain their original RHS request
 * and finite source chain, even beneath a non-filesystem placeholder head.
 */
static void it_reports_exact_nested_static_requester_chains_and_root_failures(void) {
    const char *definitions[] = {NULL, "unfinished module\n", "@!\n bad\n"};
    ProjectStatus expected[] = {PROJECT_MISSING, PROJECT_DRAFT, PROJECT_SYNTAX_ERROR};
    for (size_t index = 0; index < 3; index++) {
        prepare_missing_static_dependency();
        if (definitions[index] != NULL) { file("generated/static standard/absent", definitions[index]); }
        setup_project();
        ProjectStatus status = load_static_closure(false);
        if (status != expected[index]) {
            fprintf(stderr, "Static dependency scenario %zu: status %d, %s\n",
                index, status, project.error.message);
        }
        assert(status == expected[index] && !project.valid);
        assert(diagnostic_valid(&project.error) && strcmp(project.error.subject, "//absent") == 0);
        assert(project.error.related_count == 2);
        assert(strcmp(project.error.related[0].name, "/test/fixture/v1.0.0/src/middle") == 0);
        assert(layout_text_is(project.error.related[0].location.span, "//absent"));
        assert(strcmp(project.error.related[1].name, "/test/fixture/v1.0.0/src/main") == 0);
        assert(layout_text_is(project.error.related[1].location.span, "/test/fixture/v1.0.0/src/middle"));
        assert(project_find(&project, "/test/fixture/v1.0.0/src/main") == NULL);
        destroy_project();
    }
}
