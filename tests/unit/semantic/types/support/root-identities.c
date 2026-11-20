/** @file
 * @brief Exhaust nominal transfer forms across distinct physical root families.
 */
/** Equal path components in project and standard roots remain distinct throughout all sixteen form pairs. */
static void it_keeps_root_kind_identity_in_the_complete_transfer_matrix(void) {
    prepare_types();
    directory("generated/standard");
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    for (size_t index = 0; index < 4; index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/standard/value%s", suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, index == 0 ? "@C\n" : index == 3
            ? "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] position\n" : "/* typed native fixture */\n");
    }
    file("generated/standard/main", "@!\n");
    const char *forms[] = {"", "&", "[]", "&[]"};
    for (size_t destination = 0; destination < 4; destination++) {
        for (size_t source = 0; source < 4; source++) {
            char text[512];
            int length = snprintf(text, sizeof(text),
                "@!\nsource%s\n.value\ndestination%s\n..value\ndestination%s\n source%s\n",
                forms[source], forms[destination], forms[destination], forms[source]);
            assert(length > 0 && (size_t)length < sizeof(text));
            file("generated/program/main", text);
            setup_semantic();
            ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
                {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
            ProjectStatus loaded = project_load(&project, roots, 2, ".main");
            assert(loaded == PROJECT_OK);
            semantic_program_init(&semantic, &project, sources.allocator);
            SemanticStatus status = semantic_fixture_names(&semantic);
            if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
            if (status == SEMANTIC_OK) { status = semantic_resolve(&semantic); }
            assert(status == SEMANTIC_INVALID && diagnostic_valid(&semantic.error));
            destroy_semantic();
        }
    }
    file("generated/program/main", "@!\nreference&\n..main\n &\n");
    setup_semantic();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, ".main");
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_fixture_names(&semantic);
    if (status == SEMANTIC_OK) { status = semantic_native(&semantic); }
    if (status == SEMANTIC_OK) { status = semantic_resolve(&semantic); }
    assert(status == SEMANTIC_INVALID && diagnostic_valid(&semantic.error));
    destroy_semantic();
}
