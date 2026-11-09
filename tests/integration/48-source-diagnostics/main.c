#include "../common.c"

static void expect_c_diagnostic_at(const char* root, const char* relative_source, unsigned int line) {
    char* compile[] = {AMA_COMPILER, (char*)root, "main", "generated/program.c", NULL};
    int status = run_process(compile, "generated/ama.out", "generated/ama.err");
    expect_status(status, 0, "generated/ama.err");
    char* check[] = {TEST_CC, "-std=c11", "-fsyntax-only", "-Dbreak=ama_source_diagnostic_missing", "-Igenerated", "generated/program.c", NULL};
    status = run_process(check, "generated/cc.out", "generated/cc.err");
    expect_status(status, 1, "generated/cc.err");
    FILE* diagnostic = fopen("generated/cc.err", "rb");
    assert(diagnostic != NULL);
    char text[16384];
    size_t length = fread(text, 1, sizeof(text) - 1, diagnostic);
    assert(!ferror(diagnostic));
    text[length] = '\0';
    close_file(diagnostic);
    char working_directory[4096];
    char* directory = getcwd(working_directory, sizeof(working_directory));
    assert(directory != NULL);
    char expected[8192];
    snprintf(expected, sizeof(expected), "%s/%s:%u:", working_directory, relative_source, line);
    if (!strstr(text, expected)) fprintf(stderr, "missing source location '%s' in:\n%s", expected, text);
    assert(strstr(text, expected) != NULL);
}

static void it_reports_a_nested_unicode_module_at_its_original_statement(void) {
    setup_test();
    expect_c_diagnostic_at("project", "project/\u6a21\u5757", 6);
    destroy_test();
}

static void it_escapes_quotes_and_backslashes_in_absolute_source_paths(void) {
    setup_test();
    expect_c_diagnostic_at("project\"\\\u6e90", "project\"\\\u6e90/main", 6);
    destroy_test();
}

void run(void) {
    it_reports_a_nested_unicode_module_at_its_original_statement();
    it_escapes_quotes_and_backslashes_in_absolute_source_paths();
}

int main(void) {
    run();
    printf("48-source-diagnostics: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
