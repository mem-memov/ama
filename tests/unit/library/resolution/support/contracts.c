/** @file
 * @brief Check exact selected exports and complete scalar/list transport contracts.
 */
/** Both selected roots share one typed graph; invalid unselected code stays outside it. */
static void it_resolves_explicit_exports_and_complete_owned_schemas(void) {
    prepare_project();
    setup_test();
    LibraryBuild build;
    bool valid = resolve(&build, records);
    if (!valid) { fprintf(stderr, "%s\n", build.error.message); }
    assert(valid && build.valid && build.frontend.valid && build.frontend.inputs.sealed);
    assert(build.export_count == 2 && build.exports->next != NULL);
    assert(project_find(&build.frontend.project, "/test/fixture/v1.0.0/src/unselected") == NULL);
    assert(build.bindings->field->role_ordinal == 0 && build.bindings->schema->kind == LIBRARY_TEXT);
    assert(build.bindings->next->schema->kind == LIBRARY_FAILURE);
    assert(build.schemas->next->next->element == build.schemas);
    FILE *first = tmpfile(), *second = tmpfile();
    assert(first != NULL && second != NULL);
    bool written = library_manifest_write(&build, first) && library_manifest_write(&build, second);
    assert(written);
    int seek_a = fseek(first, 0, SEEK_SET), seek_b = fseek(second, 0, SEEK_SET);
    assert(seek_a == 0 && seek_b == 0);
    char a[8192], b[8192];
    size_t a_size = fread(a, 1, sizeof(a) - 1, first), b_size = fread(b, 1, sizeof(b) - 1, second);
    assert(a_size < sizeof(a) - 1 && a_size == b_size && memcmp(a, b, a_size) == 0);
    a[a_size] = 0;
    assert(strstr(a, "\"profile\":\"failure-v1\"") != NULL && strstr(a, "\"encoder\":") != NULL);
    assert(strstr(a, "\"calling_convention\":\"c\"") != NULL && strstr(a, "unselected") == NULL);
    FILE *full = fopen("/dev/full", "wb");
    assert(full != NULL);
    written = library_manifest_write(&build, full);
    assert(!written);
    int closed_full = fclose(full);
    (void)closed_full;
    int closed_a = fclose(first), closed_b = fclose(second);
    assert(closed_a == 0 && closed_b == 0);
    library_build_destroy(&build);
    destroy_test();
}
/** Diagnose missing exports, unsupported transports, wrong forms and incomplete/duplicate bindings. */
static void it_rejects_missing_and_incompatible_explicit_contracts(void) {
    const char *invalid[] = {
        "export | run | /test/fixture/v1.0.0/src/missing | entry\n",
        "export | run | /test/fixture/v1.0.0/src/constants | read\n",
        "export | run | /test/fixture/v1.0.0/src/main | entry\n",
        "schema | text | text | //integer\nexport | run | /test/fixture/v1.0.0/src/main | entry\n",
        "schema | text | text | //text\nexport | run | /test/fixture/v1.0.0/src/main | read\ninput | run | message text | text\n",
        "schema | text | text | //text\nexport | run | /test/fixture/v1.0.0/src/main | entry\ninput | run | unknown | text\n",
        "schema | text | text | //text\nexport | run | /test/fixture/v1.0.0/src/main | entry\ninput | run | message text | text\ninput | run | message text | text\n",
        "schema | text | text | //text\nschema | list | list | missing\nexport | run | /test/fixture/v1.0.0/src/main | read\n",
        "schema | list | list | list\nexport | run | /test/fixture/v1.0.0/src/main | read\n"
    };
    prepare_project();
    file("generated/project/packages/test/fixture/v1.0.0/src/constants", "@=\ntruth\n//std/condition\n |1\n");
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        LibraryBuild build;
        bool valid = resolve(&build, invalid[index]);
        assert(!valid && !build.valid && diagnostic_valid(&build.error));
        library_build_destroy(&build);
        destroy_test();
    }
    file("generated/project/packages/test/fixture/v1.0.0/src/reference", "@>\ntext&\n//text\n@!\n");
    setup_test();
    LibraryBuild build;
    bool valid = resolve(&build, "schema | text | text | //text\nexport | run | /test/fixture/v1.0.0/src/reference | read\noutput | run | text | text\n");
    assert(!valid && strstr(build.error.message, "owned transport") != NULL);
    library_build_destroy(&build);
    destroy_test();
}

/** A completed library refuses metadata rebinding to a different valid snapshot without losing its artifact contract. */
static void it_rejects_rebinding_a_completed_library_model(void) {
    prepare_project();
    setup_test();
    LibraryBuild build;
    assert(resolve(&build,records) && build.valid);
    const SemanticType *original_type = build.exports->type;
    const SemanticSymbol *original_field = build.bindings->field;
    CompilerFrontend empty;
    compiler_frontend_init(&empty,sources.allocator);
    assert(module_catalog_snapshot(&empty.project.catalog,NULL,0) == MODULE_OK);
    assert(compiler_frontend_check_snapshot(&empty,NULL,0));
    bool rebound = library_configuration_validate(&build,&empty.semantic);
    assert(!rebound && build.valid && build.error.message == NULL);
    assert(build.exports->type == original_type && build.bindings->field == original_field);
    FILE *manifest = tmpfile();
    assert(manifest != NULL);
    assert(library_manifest_write(&build,manifest));
    assert(fclose(manifest) == 0);
    compiler_frontend_destroy(&empty);
    library_build_destroy(&build);
    destroy_test();
}
