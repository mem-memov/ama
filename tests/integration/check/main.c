/**
 * @file
 * @brief Run actual check/status/emission commands without accidental native execution.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include <sys/stat.h>
#include "../common.c"

/**
 * Assert that the component checks a real project without c compilation or output artifacts. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_checks_a_real_project_without_c_compilation_or_output_artifacts(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", "/test/fixture/v1.0.0/src/main", NULL};
    int result = run_compiler_arguments(arguments);
    char text[4096];
    read_stream(output, text, sizeof(text));
    assert(result == EXIT_SUCCESS && strcmp(text, "checked /test/fixture/v1.0.0/src/main\n") == 0);
    read_stream(errors, text, sizeof(text));
    assert(text[0] == 0);
    struct stat metadata;
    int found = stat("project/packages/test/fixture/v1.0.0/src/generated", &metadata);
    assert(found == -1 && errno == ENOENT);
    destroy_test();
}

/**
 * Assert that the component reports exact missing modules and malformed options. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_reports_exact_missing_modules_and_malformed_options(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", "/test/fixture/v1.0.0/src/missing", NULL};
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

/**
 * Assert that the component reports drafts distinctly without accepting them as required
 * modules. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_reports_drafts_distinctly_without_accepting_them_as_required_modules(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--status", "project", "/test/fixture/v1.0.0/src/draft", NULL};
    int result = run_compiler_arguments(arguments);
    char text[4096];
    read_stream(output, text, sizeof(text));
    assert(result == EXIT_SUCCESS && strcmp(text, "draft /test/fixture/v1.0.0/src/draft\n") == 0);
    destroy_test();
    setup_test();
    arguments[1] = "--check";
    result = run_compiler_arguments(arguments);
    read_stream(errors, text, sizeof(text));
    assert(result != EXIT_SUCCESS && strstr(text, "inactive draft") != NULL);
    destroy_test();
}

/**
 * Assert that the component emits from the same frontend without invoking the native c compiler.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_emits_from_the_same_frontend_without_invoking_the_native_c_compiler(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--emit", "project", "/test/fixture/v1.0.0/src/main", "--out", "generated/assembly", "--sdk", TEST_SDK, "--mode", "executable", "--component", "test.application", NULL};
    int result = run_compiler_arguments(arguments);
    char text[4096];
    read_stream(output, text, sizeof(text));
    assert(result == EXIT_SUCCESS && strcmp(text, "emitted project assembly /test/fixture/v1.0.0/src/main\n") == 0);
    struct stat metadata;
    int found = stat("generated/assembly/launcher.c", &metadata);
    assert(found == 0 && S_ISREG(metadata.st_mode));
    destroy_test();
}

/** Both commands must reject the same frontend defect before any native toolchain or output publication. */
static void it_matches_check_and_verified_compile_diagnostics_without_publishing_failed_projects(void) {
    const char *entries[] = {"/test/fixture/v1.0.0/src/missing", "/test/fixture/v1.0.0/src/draft"};
    for (size_t index = 0; index < sizeof(entries) / sizeof(*entries); index++) {
        setup_test();
        char *checked[] = {AMA_EXECUTABLE, "--check", "project", (char *)entries[index], NULL};
        int result = run_compiler_arguments(checked);
        char expected[4096], acknowledgement[512];
        read_stream(errors, expected, sizeof(expected));
        read_stream(output, acknowledgement, sizeof(acknowledgement));
        assert(result == EXIT_FAILURE && expected[0] != 0 && acknowledgement[0] == 0);
        destroy_test();
        setup_test();
        char *compiled[] = {AMA_EXECUTABLE, "--emit", "project", (char *)entries[index],
            "--out", "generated/rejected", "--sdk", TEST_SDK, "--compile", "--mode", "executable", "--component", "test.application", NULL};
        result = run_compiler_arguments(compiled);
        char actual[4096];
        read_stream(errors, actual, sizeof(actual));
        read_stream(output, acknowledgement, sizeof(acknowledgement));
        assert(result == EXIT_FAILURE && strcmp(expected, actual) == 0 && acknowledgement[0] == 0);
        struct stat metadata;
        int found = stat("generated/rejected", &metadata);
        assert(found == -1 && errno == ENOENT);
        destroy_test();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_matches_check_and_verified_compile_diagnostics_without_publishing_failed_projects();
    it_checks_a_real_project_without_c_compilation_or_output_artifacts();
    it_emits_from_the_same_frontend_without_invoking_the_native_c_compiler();
    it_reports_exact_missing_modules_and_malformed_options();
    it_reports_drafts_distinctly_without_accepting_them_as_required_modules();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
