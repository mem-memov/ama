/** @file
 * @brief Partition valid, malformed, duplicate and unsupported explicit library records.
 */
#include "../common.c"
/** Parse records without module discovery; preserve a multiword field and arbitrary flag bytes. */
static void it_preserves_explicit_exports_schemas_and_settings(void) {
    setup_test();
    const SourceText *source = configuration("schema | text | text | ..text\nexport | run | .module with spaces | entry,read\ninput | run | message text | text\nc-flag | -DVALUE=A | B\n");
    LibraryBuild build;
    library_build_init(&build, sources.allocator);
    bool parsed = library_configuration_parse(&build, source);
    assert(parsed && build.parsed && !build.valid && !build.frontend.attempted);
    assert(build.export_count == 1 && strcmp(build.exports->module, ".module with spaces") == 0);
    assert(build.exports->operations == (LIBRARY_ENTRY | LIBRARY_READ));
    assert(strcmp(build.bindings->field_name, "message text") == 0);
    assert(strcmp(build.settings->value, "-DVALUE=A | B") == 0);
    library_build_destroy(&build);
    destroy_test();
}
/** Unknown, repeated and incomplete records cannot silently select a default interpretation. */
static void it_rejects_the_complete_record_error_partitions(void) {
    const char *invalid[] = {
        "", "export | run | .main | entry\ncomponent | duplicate\n",
        "export | run | .main | entry,entry\n", "export | run | .main | entry,\n",
        "export | run | .main | unknown\n", "export | _reserved | .main | entry\n",
        "export | run | main | entry\n", "export | run | .main | entry\nexport | run | .other | read\n",
        "export | run | .main | entry\nschema | value | opaque | .private\n",
        "export | run | .main | entry\nschema | text | text | ..text\nschema | text | text | ..text\n",
        "export | run | .main | entry\nunknown | data\n", "export | run | .main | entry\nabi | other\n",
        "export | run | .main | entry\ninput | run | field\n", "export | run | .main | entry\ninput | run |  | text\n",
        "export | run | .main | entry\ninput | run | field | text \n",
        "export | run | .main | entry\n# forbidden\ttab\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        const SourceText *source = configuration(invalid[index]);
        LibraryBuild build;
        library_build_init(&build, sources.allocator);
        bool parsed = library_configuration_parse(&build, source);
        assert(!parsed && !build.valid && diagnostic_valid(&build.error));
        library_build_destroy(&build);
        destroy_test();
    }
}
/** Byte validity, required metadata and edition selection have explicit rejection paths. */
static void it_rejects_malformed_bytes_and_unsupported_metadata(void) {
    const char *invalid[] = {
        "ama-library | 2\n", "component | missing header\n", "ama-library | 1\nexport | run | .main | entry\n",
        "ama-library | 1\ncomponent | x\nabi | other\ntarget | t\ncalling-convention | c\nexport | run | .main | entry\n",
        "ama-library | 1\ncomponent | x\nabi | ama-c-1\ntarget | t\ncalling-convention | stdcall\nexport | run | .main | entry\n",
        "ama-library | 1\nama-library | 1\n", "ama-library | 1\r\n", "ama-library | 1\n# \377\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        const SourceText *source = NULL;
        SourceStatus added = source_manager_add(&sources, "invalid.exports", (const unsigned char *)invalid[index], strlen(invalid[index]), &source);
        assert(added == SOURCE_OK);
        LibraryBuild build;
        library_build_init(&build, sources.allocator);
        bool parsed = library_configuration_parse(&build, source);
        assert(!parsed && !build.parsed && diagnostic_valid(&build.error));
        library_build_destroy(&build);
        destroy_test();
    }
    setup_test();
    const unsigned char bytes[] = "ama-library | 1\n# embedded\0zero\n";
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "nul.exports", bytes, sizeof(bytes) - 1, &source);
    assert(added == SOURCE_OK);
    LibraryBuild build;
    library_build_init(&build, sources.allocator);
    bool parsed = library_configuration_parse(&build, source);
    assert(!parsed && build.error.primary.span.start == 26);
    library_build_destroy(&build);
    destroy_test();
}
/** Every failed parse allocation leaves all previous records privately destructible. */
static void it_unwinds_every_configuration_allocation(void) {
    const char *tail = "schema | text | text | ..text\nexport | run | .main | entry,read\ninput | run | text | text\nlink-flag | -lm\n";
    setup_test();
    const SourceText *source = configuration(tail);
    LibraryBuild build;
    library_build_init(&build, sources.allocator);
    size_t before = allocation_count;
    bool parsed = library_configuration_parse(&build, source);
    assert(parsed);
    size_t count = allocation_count - before;
    library_build_destroy(&build);
    destroy_test();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_test();
        source = configuration(tail);
        library_build_init(&build, sources.allocator);
        failure_at = allocation_count + failure;
        parsed = library_configuration_parse(&build, source);
        assert(!parsed && !build.valid && build.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        library_build_destroy(&build);
        destroy_test();
    }
}
/** Explicitly execute all configuration boundary checks. */
void run(void) {
    it_rejects_malformed_bytes_and_unsupported_metadata();
    it_preserves_explicit_exports_schemas_and_settings();
    it_rejects_the_complete_record_error_partitions();
    it_unwinds_every_configuration_allocation();
}
/** Return only after every partial allocation and invalid record has been checked. */
int main(void) { run(); return EXIT_SUCCESS; }
