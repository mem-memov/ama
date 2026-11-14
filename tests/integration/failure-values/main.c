/** @file
 * @brief Compile and run the actual failure constructor, positive check and description reader.
 */
#include "../common.c"

/** Execute a published standard-root project with separate fresh output streams. */
static int run_failure_program(void) {
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

/** Construction creates a true value, retained read preserves its description,
 * and explicit entry resets it to false/empty without a separate result flag.
 */
static void it_executes_the_real_owned_failure_value_through_the_cli(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--emit", "project", ".main", "--standard-root", TEST_STANDARD_ROOT,
        "--out", "generated/output", "--sdk", TEST_SDK, "--compile", NULL};
    int result = run_compiler_arguments(arguments);
    char text[8192];
    read_stream(errors, text, sizeof(text));
    if (result != EXIT_SUCCESS) { fprintf(stderr, "%s", text); }
    assert(result == EXIT_SUCCESS && text[0] == '\0');
    destroy_test();
    setup_test();
    result = run_failure_program();
    read_stream(errors, text, sizeof(text));
    assert(result == EXIT_SUCCESS && text[0] == '\0');
    read_stream(output, text, sizeof(text));
    assert(strcmp(text, "description=expected application failure\ndescription=\n") == 0);
    destroy_test();
}

/** Exercise actual source loading, native verification and final program execution. */
void run(void) { it_executes_the_real_owned_failure_value_through_the_cli(); }

/** Return after exact CLI status, streams and complete outcome transitions match. */
int main(void) { run(); return EXIT_SUCCESS; }
