/** @file
 * @brief Resolve explicit standard roots and execute the actual integer/list bridge via CLI.
 */
#include "../common.c"

/** Run a successfully published executable with fresh captured streams. */
static int run_integer_program(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        execl("generated/output/program", "generated/output/program", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/** Library authority is explicit: project roots never supply double-dot modules. */
static void it_requires_an_explicit_standard_root_to_resolve_integer(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", "/test/fixture/v1.0.0/src/main", NULL};
    int result = run_compiler_arguments(arguments);
    char text[4096];
    read_stream(errors, text, sizeof(text));
    assert(result != EXIT_SUCCESS && strstr(text, "missing-module") != NULL);
    destroy_test();
    setup_test();
    char *configured[] = {AMA_EXECUTABLE, "--check", "project", "/test/fixture/v1.0.0/src/main",
        "--standard-root", TEST_STANDARD_ROOT, NULL};
    result = run_compiler_arguments(configured);
    read_stream(errors, text, sizeof(text));
    assert(result == EXIT_SUCCESS && text[0] == '\0');
    read_stream(output, text, sizeof(text));
    assert(strcmp(text, "checked /test/fixture/v1.0.0/src/main\n") == 0);
    destroy_test();
}

/** Nested selected Type constructors supply actual list element storage;
 * whole-alias constant lists retain integer payloads through verified execution.
 */
static void it_compiles_and_executes_real_integer_list_results(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--emit", "project", "/test/fixture/v1.0.0/src/main", "--standard-root", TEST_STANDARD_ROOT,
        "--out", "generated/output", "--sdk", TEST_SDK, "--compile", NULL};
    int result = run_compiler_arguments(arguments);
    char text[8192];
    read_stream(errors, text, sizeof(text));
    if (result != EXIT_SUCCESS) { fprintf(stderr, "%s", text); }
    assert(result == EXIT_SUCCESS && text[0] == '\0');
    destroy_test();
    setup_test();
    result = run_integer_program();
    read_stream(errors, text, sizeof(text));
    assert(result == EXIT_SUCCESS && text[0] == '\0');
    read_stream(output, text, sizeof(text));
    assert(strcmp(text, "length=7\nlength=3\nlength=0\n") == 0);
    destroy_test();
}

/** Exercise command parsing, root resolution, native verification and execution. */
void run(void) {
    it_requires_an_explicit_standard_root_to_resolve_integer();
    it_compiles_and_executes_real_integer_list_results();
}

/** Return success after all real commands and exact program output match. */
int main(void) { run(); return EXIT_SUCCESS; }
