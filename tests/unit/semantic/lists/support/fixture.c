/** @file
 * @brief Supply distinct project and standard roots and load intrinsic list projects without executing authored native C.
 */
/** Write an authored standard integer companion set with exactly the requested six-slot capability profile. */
static void integer_fixture(const char *interface, const char *abi) {
    directory("generated/standard");
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    for (size_t index = 0; index < 4; index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/standard/integer%s", suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, index == 0 ? interface : index == 3 ? abi : "/* native interface fixture; C signatures are checked at emission */\n");
    }
}
/** Prepare copyable/noncopyable elements and an explicit standard integer identity; project .integer is an impostor. */
static void prepare_lists(void) {
    prepare_types();
    const char *all = "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n";
    integer_fixture("@C\n", all);
    native_fixture("integer", all);
}
/** Parse the exact two-root closure and consume each semantic phase only after its predecessor succeeds. */
static SemanticStatus resolve_list(const char *text) {
    file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
    setup_semantic();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main");
    if (loaded != PROJECT_OK) { fprintf(stderr, "List fixture: %s\n%s\n", project.error.message, text); }
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_fixture_names(&semantic);
    if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(&semantic); }
    return status;
}
/** Require the exact acceptance partition and valid original diagnostics, then release all borrowed owners. */
static void expect_list(const char *text, bool expected) {
    SemanticStatus status = resolve_list(text);
    if ((status == SEMANTIC_OK) != expected) { fprintf(stderr, "List typing: %s\n%s\n", semantic.error.message, text); }
    assert((status == SEMANTIC_OK) == expected);
    if (!expected) { assert(diagnostic_valid(&semantic.error)); }
    destroy_semantic();
}
/** Find one exact intrinsic entry in the caller's typed inventory; no source name becomes a module instance. */
static SemanticStatement *list_method(SemanticListMethod method) {
    for (SemanticStatement *statement = semantic_entry()->statements; statement != NULL; statement = statement->next) {
        if (statement->syntax->kind == AST_CALL && statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION
            && statement->list_method == method) { return statement; }
    }
    return NULL;
}
