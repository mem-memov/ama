#include "../common.c"

static void export_types(const char* project, const char* directory, int expected) {
    char* arguments[] = {AMA_COMPILER, "--generate-type-layouts", (char*)project, (char*)directory, NULL};
    int status = run_process(arguments, "generated/export.out", "generated/export.err");
    expect_status(status, expected, "generated/export.err");
}

static void it_exports_a_real_typed_project_and_runs_the_generated_c(void) {
    setup_test();
    export_types("project/valid", "generated/types", 0);
    char* compiler[] = {TEST_CC, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-Igenerated/types", "-I" TEST_ROOT "/build/stdlib/include",
        "generated/types/ama_types.c", "project/valid/native/buffer.c", "project/driver.c",
        AMA_STANDARD_LIBRARY_ARCHIVE, "-o", "generated/program", NULL};
    int status = run_process(compiler, "generated/cc.out", "generated/cc.err");
    expect_status(status, 0, "generated/cc.err");
    char* program[] = {"./generated/program", NULL};
    status = run_process(program, "generated/stdout", "generated/stderr");
    expect_status(status, 0, "generated/stderr");
    assert_files_equal("generated/stdout", "expected.txt");
    export_types("project/valid", "generated/types", 0);
    destroy_test();
}

static void it_rejects_invalid_projects_without_creating_output(void) {
    setup_test();
    const char* projects[] = {"project/unknown", "project/cycle", "project/draft-type",
        "project/bad-syntax", "project/no-provider", "project/references", "project/only-draft",
        "project/no-source"};
    const char* messages[] = {"unknown module type", "recursive inline ownership", "no implementation annotation",
        "unexpected indent", "requires layout header", "owned scalar", "no implemented typed modules",
        "requires companion C file"};
    for (size_t index = 0; index < sizeof(projects) / sizeof(projects[0]); index++) {
        export_types(projects[index], "generated/rejected", 1);
        FILE* diagnostic = fopen("generated/export.err", "rb");
        assert(diagnostic);
        char text[4096];
        size_t length = fread(text, 1, sizeof(text) - 1, diagnostic);
        text[length] = '\0';
        assert(!ferror(diagnostic) && strstr(text, messages[index]));
        close_file(diagnostic);
        assert(access("generated/rejected/ama_types.h", F_OK) != 0);
        assert(access("generated/rejected/ama_types.c", F_OK) != 0);
    }
    destroy_test();
}

static void it_keeps_layout_export_independent_of_the_numeric_library_environment(void) {
    setup_test();
    const char* previous = getenv("AMA_STDLIB");
    char* saved = previous ? strdup(previous) : NULL;
    assert(!previous || saved);
    assert(setenv("AMA_STDLIB", "generated/no-such-numeric-library", 1) == 0);
    export_types("project/valid", "generated/without-numeric-library", 0);
    if (saved) {
        assert(setenv("AMA_STDLIB", saved, 1) == 0);
        free(saved);
    } else {
        assert(unsetenv("AMA_STDLIB") == 0);
    }
    destroy_test();
}

void run(void) {
    it_exports_a_real_typed_project_and_runs_the_generated_c();
    it_rejects_invalid_projects_without_creating_output();
    it_keeps_layout_export_independent_of_the_numeric_library_environment();
}

int main(void) {
    run();
    printf("57-typed-layouts: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
