/** @file
 * @brief Build the maintained transitive Ama/native project through the actual CLI and verify exact state, return-routing and cleanup effects.
 */
#include <stdbool.h>
#include "../common.c"

/** Execute only the published program, with explicit optional sanitizer policy and captured streams; never inspect generated source or binary contents. */
static int run_minimal(const char *program, bool sanitized) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        if (sanitized && (setenv("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1", 1) != 0
            || setenv("UBSAN_OPTIONS", "halt_on_error=1", 1) != 0)) { _exit(126); }
        execl(program, program, (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/** Compare maintained expected effects only after verified compilation; separate streams prevent compiler acknowledgements from becoming program output. */
static void compile_and_run_minimal(bool sanitized) {
    setup_test();
    char *ordinary[] = {AMA_EXECUTABLE, "--emit", "project", ".main", "--out", "generated/ordinary", "--sdk", TEST_SDK, "--compile", NULL};
    char *instrumented[] = {AMA_EXECUTABLE, "--emit", "project", ".main", "--out", "generated/sanitized", "--sdk", TEST_SDK, "--compile",
        "--c-flag", "-fsanitize=address,undefined", "--c-flag", "-fno-omit-frame-pointer", "--c-flag", "-fno-pie", "--c-flag", "-no-pie", NULL};
    int result = run_compiler_arguments(sanitized ? instrumented : ordinary);
    char diagnostic[8192];
    read_stream(errors, diagnostic, sizeof(diagnostic));
    if (result != EXIT_SUCCESS) { fprintf(stderr, "Minimal compilation failed: %s\n", diagnostic); }
    assert(result == EXIT_SUCCESS && diagnostic[0] == '\0');
    destroy_test();
    setup_test();
    result = run_minimal(sanitized ? "generated/sanitized/program" : "generated/ordinary/program", sanitized);
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert(result == EXIT_SUCCESS && diagnostic[0] == '\0');
    char actual[4096], expected[4096];
    read_stream(output, actual, sizeof(actual));
    FILE *expectation = fopen("expected.txt", "rb");
    assert(expectation != NULL);
    read_stream(expectation, expected, sizeof(expected));
    int closed = fclose(expectation);
    assert(closed == 0);
    if (strcmp(actual, expected) != 0) { fprintf(stderr, "Unexpected minimal effects:\n%s\n", actual); }
    assert(strcmp(actual, expected) == 0);
    destroy_test();
}

/** The project closure and native contracts must check without invoking C or executing native bodies. */
static void it_checks_the_complete_minimal_project(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", ".main", NULL};
    int result = run_compiler_arguments(arguments);
    char acknowledgement[128], diagnostic[1024];
    read_stream(output, acknowledgement, sizeof(acknowledgement));
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert(result == EXIT_SUCCESS && strcmp(acknowledgement, "checked .main\n") == 0 && diagnostic[0] == '\0');
    destroy_test();
}

/** Actual verified compilation must preserve independent native state and release every successfully constructed resource. */
static void it_runs_the_minimal_project_with_stable_growth_returns_and_complete_cleanup(void) {
    compile_and_run_minimal(false);
}

/** Instrument the actual generated program, including runtime and native companions; resource-credit observations complement address/undefined-behavior checks. */
static void it_runs_the_minimal_project_under_address_and_undefined_behavior_sanitizers(void) {
    compile_and_run_minimal(true);
}

/** Select every independent acceptance case explicitly. */
void run(void) {
    it_checks_the_complete_minimal_project();
    it_runs_the_minimal_project_with_stable_growth_returns_and_complete_cleanup();
    it_runs_the_minimal_project_under_address_and_undefined_behavior_sanitizers();
}

/** Return success only after all check, compile, execution and cleanup observations pass. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
