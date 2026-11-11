#include <assert.h>
#include <stdlib.h>

#include "driver/request/request.h"

static void it_handles_an_empty_vector_without_reading_arguments(void) {
    DriverRequest request = driver_parse_request(0, NULL);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
}

static void it_accepts_only_a_standalone_help_request(void) {
    char *arguments[] = {"ama", "--help"};
    DriverRequest request = driver_parse_request(2, arguments);
    assert(request == DRIVER_REQUEST_HELP);
}

static void it_does_not_guess_help_from_a_prefix_or_extra_arguments(void) {
    char *arguments[] = {"ama", "--help", "project"};
    DriverRequest request = driver_parse_request(3, arguments);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
    arguments[1] = "--hel";
    request = driver_parse_request(2, arguments);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
}

static void it_refuses_all_unimplemented_operations(void) {
    char *arguments[] = {"ama", "project", "extra"};
    for (int argument_count = 1; argument_count <= 3; argument_count++) {
        DriverRequest request = driver_parse_request(argument_count, arguments);
        assert(request == DRIVER_REQUEST_UNAVAILABLE);
    }
    arguments[1] = "--check";
    DriverRequest request = driver_parse_request(2, arguments);
    assert(request == DRIVER_REQUEST_UNAVAILABLE);
}

void run(void) {
    it_handles_an_empty_vector_without_reading_arguments();
    it_accepts_only_a_standalone_help_request();
    it_does_not_guess_help_from_a_prefix_or_extra_arguments();
    it_refuses_all_unimplemented_operations();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
