/** @file
 * @brief Validate the published C example workflow, metadata contract and safe output failures.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires actual
 * independently compiled consumers, not merely generated declarations.
 */
#include "../common.c"
/** Run the real Python entry point, preserving bounded observations and the child's exit status. */
static int example(const char *destination, const char *compiler) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        int redirected = dup2(fileno(output), STDOUT_FILENO);
        int errors_redirected = dup2(fileno(errors), STDERR_FILENO);
        if (redirected < 0 || errors_redirected < 0) { _exit(127); }
        execlp("python3", "python3", TEST_EXAMPLES "/run.py", "--out", destination,
            "--compiler", compiler, "--cc", TEST_C_COMPILER,
#ifdef TEST_SANITIZERS
            "--sanitize",
#endif
            (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status));
    return WEXITSTATUS(status);
}
/** Four executables validate transport ownership, native resource balance, yields and failure reset. */
static void it_builds_and_runs_static_and_shared_multifile_consumers(void) {
    char observations[8192], diagnostic[8192];
    setup_test();
    int result = example("generated/example with spaces", AMA_EXECUTABLE);
    read_stream(output, observations, sizeof(observations));
    read_stream(errors, diagnostic, sizeof(diagnostic));
    if (result != 0) { fputs(diagnostic, stderr); }
    assert(result == 0);
    const char *standalone = "standalone: 2 yields, independent retained value";
    const char *nested = "nested: explicit yields, supervised failure, explicit reset";
    const char *first = strstr(observations, standalone);
    assert(first != NULL && strstr(first + strlen(standalone), standalone) != NULL);
    first = strstr(observations, nested);
    assert(first != NULL && strstr(first + strlen(nested), nested) != NULL);
    destroy_test();
}
/** Fresh output is mandatory; neither an existing tree nor a failing tool is silently overwritten. */
static void it_preserves_existing_outputs_and_reports_failed_tools(void) {
    char diagnostic[8192];
    setup_test();
    int result = example("generated/example with spaces", AMA_EXECUTABLE);
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert(result != 0 && strstr(diagnostic, "File exists") != NULL);
    destroy_test();
    setup_test();
    result = example("generated/failed", "/bin/false");
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert(result != 0 && strstr(diagnostic, "non-zero exit status") != NULL);
    destroy_test();
}
/** Exercise the metadata consumer's exact identities, filenames and command record independently. */
static void it_checks_metadata_bindings_and_rejects_malformed_models(void) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        execlp("python3", "python3", "verify.py", TEST_EXAMPLES,
            "generated/example with spaces", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Register complete example acceptance and failure containment through the common C harness. */
void run(void) {
    it_builds_and_runs_static_and_shared_multifile_consumers();
    it_preserves_existing_outputs_and_reports_failed_tools();
    it_checks_metadata_bindings_and_rejects_malformed_models();
}
/** Return only after all actual compiled C consumers and metadata checks have succeeded. */
int main(void) { run(); return EXIT_SUCCESS; }
