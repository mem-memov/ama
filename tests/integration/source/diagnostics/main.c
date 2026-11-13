/**
 * @file
 * @brief Compose exact Unicode source identity, path diagnostics and C source mapping.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component composes source lookup identity spans and human diagnostics. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_composes_source_lookup_identity_spans_and_human_diagnostics(void) {
    setup_test();
    ModuleRoot root = {"project", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus discovered = module_catalog_build(&catalog, &root, 1);
    assert(discovered == MODULE_OK);
    const ModuleSource *module;
    discovered = module_catalog_find(&catalog, ".商店.商品", &module);
    assert(discovered == MODULE_OK);
    QualifiedIdentity identity = {IDENTITY_TYPE, module->logical_path, NULL, 0,
        {module->text, 0, 2}};
    const IdentityName *name;
    IdentityStatus registered = identity_register(&identities, identity, &name);
    assert(registered == IDENTITY_OK);
    Diagnostic diagnostic = {DIAGNOSTIC_SYNTAX, "Example source context", name->source_name,
        {name->declaration, NULL}, NULL, NULL, NULL, NULL, 0};
    expect_diagnostic(&diagnostic, "expected.txt");
    destroy_test();
}

/**
 * Assert that the component reports real source ambiguity with both physical paths. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_reports_real_source_ambiguity_with_both_physical_paths(void) {
    setup_test();
    ModuleRoot roots[] = {{"project/商店", NULL, 0, MODULE_ROOT_PROJECT}, {"project/other", NULL, 0, MODULE_ROOT_PROJECT}};
    ModuleStatus discovered = module_catalog_build(&catalog, roots, 2);
    assert(discovered == MODULE_AMBIGUOUS);
    Diagnostic diagnostic;
    DiagnosticRelated related;
    bool adapted = diagnostic_from_module(&catalog.issue, &related, &diagnostic);
    assert(adapted);
    expect_diagnostic(&diagnostic, "expected-ambiguity.txt");
    destroy_test();
}

/**
 * Assert that the component renders an actual invalid utf8 input without losing its bytes. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_renders_an_actual_invalid_utf8_input_without_losing_its_bytes(void) {
    setup_test();
    int created = mkdir("generated/bad", 0700);
    assert(created == 0 || errno == EEXIST);
    FILE *input = fopen("generated/bad/坏", "wb");
    assert(input != NULL);
    const unsigned char bytes[] = {'@', '!', '\n', 0xff};
    size_t written = fwrite(bytes, 1, sizeof(bytes), input);
    int closed = fclose(input);
    assert(written == sizeof(bytes) && closed == 0);
    ModuleRoot root = {"generated/bad", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus discovered = module_catalog_build(&catalog, &root, 1);
    assert(discovered == MODULE_INVALID_UTF8);
    Diagnostic diagnostic;
    DiagnosticRelated related;
    bool adapted = diagnostic_from_module(&catalog.issue, &related, &diagnostic);
    assert(adapted);
    expect_diagnostic(&diagnostic, "expected-error.txt");
    destroy_test();
}

/**
 * Assert that the component compiles line mappings with unicode quotes controls and backslashes.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_compiles_line_mappings_with_unicode_quotes_controls_and_backslashes(void) {
    setup_test();
    const char *path = "商店/\"item\\name\t\n" "?" "?/";
    const SourceText *source;
    SourceStatus added = source_manager_add(&catalog.sources, path,
        (const unsigned char *)"a\nb", 3, &source);
    assert(added == SOURCE_OK);
    FILE *output = fopen("generated/mapping.c", "wb");
    assert(output != NULL);
    int written = fputs("#include <string.h>\n", output);
    assert(written != EOF);
    SourceStatus mapped = source_write_line_directive(output, source, 2);
    assert(mapped == SOURCE_OK);
    written = fputs("int main(void) { const unsigned char expected[] = {", output);
    assert(written != EOF);
    for (const unsigned char *byte = (const unsigned char *)path; *byte != 0; byte++) {
        written = fprintf(output, "%u,", (unsigned int)*byte);
        assert(written >= 0);
    }
    written = fputs("0}; return __LINE__ != 2 || strcmp(__FILE__, (const char *)expected) != 0; }\n", output);
    assert(written != EOF);
    int closed = fclose(output);
    assert(closed == 0);
    char *compiler[] = {TEST_REAL_COMPILER, "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror",
        "-finput-charset=UTF-8", "-fexec-charset=UTF-8",
        "generated/mapping.c", "-o", "generated/mapping", NULL};
    run_process(compiler);
    char *program[] = {"./generated/mapping", NULL};
    run_process(program);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_composes_source_lookup_identity_spans_and_human_diagnostics();
    it_reports_real_source_ambiguity_with_both_physical_paths();
    it_renders_an_actual_invalid_utf8_input_without_losing_its_bytes();
    it_compiles_line_mappings_with_unicode_quotes_controls_and_backslashes();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
