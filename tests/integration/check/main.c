#include <sys/stat.h>
#include "../common.c"

static void it_checks_a_real_project_without_c_compilation_or_output_artifacts(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", ".main", NULL};
    int result = run_compiler_arguments(arguments);
    char text[4096];
    read_stream(output, text, sizeof(text));
    assert(result == EXIT_SUCCESS && strcmp(text, "checked .main\n") == 0);
    read_stream(errors, text, sizeof(text));
    assert(text[0] == 0);
    struct stat metadata;
    int found = stat("project/generated", &metadata);
    assert(found == -1 && errno == ENOENT);
    destroy_test();
}

static void it_reports_exact_missing_modules_and_malformed_options(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", ".missing", NULL};
    int result = run_compiler_arguments(arguments);
    char text[4096];
    read_stream(errors, text, sizeof(text));
    assert(result != EXIT_SUCCESS && strstr(text, "missing-module") != NULL);
    destroy_test();
    setup_test();
    arguments[3] = "not.a.logical.path";
    result = run_compiler_arguments(arguments);
    read_stream(errors, text, sizeof(text));
    assert(result != EXIT_SUCCESS && strstr(text, "exact logical module path") != NULL);
    destroy_test();
}

void run(void) {
    it_checks_a_real_project_without_c_compilation_or_output_artifacts();
    it_reports_exact_missing_modules_and_malformed_options();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
