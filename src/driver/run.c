#include "driver.h"

#include <stdlib.h>

#include "output/output.h"
#include "request/request.h"

int driver_run(int argument_count, char *const arguments[],
    FILE *output, FILE *errors) {
    DriverRequest request = driver_parse_request(argument_count, arguments);
    FILE *destination = request == DRIVER_REQUEST_HELP ? output : errors;
    int result = driver_write_response(request, destination);

    return request == DRIVER_REQUEST_HELP ? result : EXIT_FAILURE;
}
