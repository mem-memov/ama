/**
 * @file
 * @brief Check the actual compiler process help, command selection and exit status.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component exposes implementation status through the real launcher. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_exposes_implementation_status_through_the_real_launcher(void) {
    setup_test();
    int result = run_compiler("--help");
    char message[1024];
    read_stream(output, message, sizeof(message));
    assert(result == EXIT_SUCCESS);
    assert(strstr(message, "--compile") != NULL);
    read_stream(errors, message, sizeof(message));
    assert(message[0] == '\0');
    destroy_test();
}

/**
 * Assert that the component rejects ambiguous project commands. The explicit fixture setup and
 * observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_rejects_ambiguous_project_commands(void) {
    setup_test();
    int result = run_compiler("project");
    char message[1024];
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_FAILURE);
    assert(strstr(message, "expected an explicit command") != NULL);
    read_stream(output, message, sizeof(message));
    assert(message[0] == '\0');
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_exposes_implementation_status_through_the_real_launcher();
    it_rejects_ambiguous_project_commands();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
