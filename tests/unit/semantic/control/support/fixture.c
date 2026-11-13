/** @file
 * @brief Supply exact typed header chains, native predicate/index capabilities and a standard intrinsic integer fixture.
 */
/** Set up separate lexical result namespaces and native conversion contracts without invoking C. */
static void prepare_control(void) {
    prepare_types();
    file("generated/program/producer", "@<\ninput\n.value\n@>\nchild\n.worker\n@!\n");
    file("generated/program/worker", "@>\ncondition\n.value\n@!\n");
    directory("generated/standard");
    file("generated/standard/integer", "@C\n");
    file("generated/standard/integer.h", "/* authored native fixture */\n");
    file("generated/standard/integer.c", "/* authored native fixture */\n");
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
}
/** Load both exact root families and publish only a fully successful native/inference/resolution sequence. */
static SemanticStatus resolve_control(const char *text) {
    file("generated/program/main", text);
    setup_semantic();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, ".main");
    if (loaded != PROJECT_OK) { fprintf(stderr, "Control fixture: %s\n%s\n", project.error.message, text); }
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_names(&semantic);
    if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(&semantic); }
    return status;
}
/** Require the exact static acceptance partition without using runtime contents to skip a body. */
static void expect_control(const char *text, bool expected) {
    SemanticStatus status = resolve_control(text);
    if ((status == SEMANTIC_OK) != expected) { fprintf(stderr, "Control typing: %s\n%s\n", semantic.error.message, text); }
    assert((status == SEMANTIC_OK) == expected);
    if (!expected) { assert(diagnostic_valid(&semantic.error)); }
    destroy_semantic();
}
/** Locate one exact source-ordered statement kind for metadata assertions; no runtime target is selected. */
static SemanticStatement *control_statement(AstNodeKind kind, size_t occurrence) {
    for (SemanticStatement *statement = semantic.first->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind != kind) { continue; }
        if (occurrence == 0) { return statement; }
        occurrence--;
    }
    return NULL;
}

/** Require a structural control rejection before semantic initialization and release the failed closure owner. */
static void expect_control_parse_error(const char *text) {
    file("generated/program/main", text);
    setup_project();
    ProjectStatus loaded = load_root("generated/program");
    assert(loaded == PROJECT_SYNTAX_ERROR && !project.valid && diagnostic_valid(&project.error));
    destroy_project();
}
