/**
 * @file
 * @brief Verify exact explicit CLI request recognition without reading absent arguments.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <assert.h>
#include <stdlib.h>

#include "driver/request/request.h"

/**
 * Assert that the component handles an empty vector without reading arguments. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_handles_an_empty_vector_without_reading_arguments(void) {
    DriverRequest request = driver_parse_request(0, NULL);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
}

/**
 * Assert that the component accepts only a standalone help request. The explicit fixture setup
 * and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_accepts_only_a_standalone_help_request(void) {
    char *arguments[] = {"ama", "--help"};
    DriverRequest request = driver_parse_request(2, arguments);
    assert(request == DRIVER_REQUEST_HELP);
}

/**
 * Assert that the component does not guess help from a prefix or extra arguments. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_does_not_guess_help_from_a_prefix_or_extra_arguments(void) {
    char *arguments[] = {"ama", "--help", "project"};
    DriverRequest request = driver_parse_request(3, arguments);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
    arguments[1] = "--hel";
    request = driver_parse_request(2, arguments);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
}

/**
 * Assert that the component refuses all unimplemented operations. The explicit fixture setup and
 * observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_refuses_all_unimplemented_operations(void) {
    char *arguments[] = {"ama", "project", "extra"};
    for (int argument_count = 1; argument_count <= 3; argument_count++) {
        DriverRequest request = driver_parse_request(argument_count, arguments);
        assert(request == DRIVER_REQUEST_UNAVAILABLE);
    }
    arguments[1] = "--check";
    DriverRequest request = driver_parse_request(2, arguments);
    assert(request == DRIVER_REQUEST_CHECK);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_handles_an_empty_vector_without_reading_arguments();
    it_accepts_only_a_standalone_help_request();
    it_does_not_guess_help_from_a_prefix_or_extra_arguments();
    it_refuses_all_unimplemented_operations();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
