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
    assert(project_find(&build.frontend.project, ".unselected") == NULL);
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
        "export | run | .missing | entry\n",
        "export | run | .constants | read\n",
        "export | run | .main | entry\n",
        "schema | text | text | ..integer\nexport | run | .main | entry\n",
        "schema | text | text | ..text\nexport | run | .main | read\ninput | run | message text | text\n",
        "schema | text | text | ..text\nexport | run | .main | entry\ninput | run | unknown | text\n",
        "schema | text | text | ..text\nexport | run | .main | entry\ninput | run | message text | text\ninput | run | message text | text\n",
        "schema | text | text | ..text\nschema | list | list | missing\nexport | run | .main | read\n",
        "schema | list | list | list\nexport | run | .main | read\n"
    };
    prepare_project();
    file("generated/project/constants", "@=\ntruth\n..std.condition\n |1\n");
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        LibraryBuild build;
        bool valid = resolve(&build, invalid[index]);
        assert(!valid && !build.valid && diagnostic_valid(&build.error));
        library_build_destroy(&build);
        destroy_test();
    }
    file("generated/project/reference", "@>\ntext&\n..text\n@!\n");
    setup_test();
    LibraryBuild build;
    bool valid = resolve(&build, "schema | text | text | ..text\nexport | run | .reference | read\noutput | run | text | text\n");
    assert(!valid && strstr(build.error.message, "owned transport") != NULL);
    library_build_destroy(&build);
    destroy_test();
}
