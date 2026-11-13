/** @file
 * @brief Create noncopyable method results and distinct caller/parent interfaces for exact selection assertions.
 */
/** A method's owned native resource forbids copying but must not forbid executing the selected result in place. */
static void prepare_selection(void) {
    prepare_types();
    file("generated/program/worker", "@<\ninput\n.value\n@>\noutput\n.value\n@!\nresource\n.resource\n");
    file("generated/program/producer", "@=\nconstant\n.value\n@<\nparameter\n.value\n@>\nmethod\n.worker\nalias&\n.worker\nmethods[]\n.worker\nreferences&[]\n.worker\nmarker\n.value\n@!\nprivate\n.worker\n");
}
/** Find an existing typed operation by its exact source target spelling and occurrence, never by dynamic target/body lookup. */
static SemanticStatement *selected_call(const char *name, size_t occurrence) {
    for (SemanticStatement *statement = semantic.first->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != AST_CALL || statement->syntax->as.call.target == NULL
            || !layout_text_is(statement->syntax->as.call.target->name, name)) { continue; }
        if (occurrence == 0) { return statement; }
        occurrence--;
    }
    return NULL;
}

/** Load one exact selected-tree fixture with a diagnostic before a fixture setup assertion; no production failure is treated as success. */
static SemanticStatus resolve_selected(const char *text) {
    file("generated/program/main", text);
    setup_semantic();
    ProjectStatus loaded = load_root("generated/program");
    if (loaded != PROJECT_OK) { fprintf(stderr, "Selection fixture: %s\n%s\n", project.error.message, text); }
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_names(&semantic);
    if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(&semantic); }
    return status;
}
/** Require the exact semantic success partition and complete owner teardown for each target/mode shape. */
static void expect_selected(const char *text, bool expected) {
    SemanticStatus status = resolve_selected(text);
    if ((status == SEMANTIC_OK) != expected) { fprintf(stderr, "Selected typing: %s\n%s\n", semantic.error.message, text); }
    assert((status == SEMANTIC_OK) == expected);
    if (!expected) { assert(diagnostic_valid(&semantic.error)); }
    destroy_semantic();
}
