#include "driver.h"

#include <stdlib.h>

#include "output/output.h"
#include "request/request.h"
#include "commands/commands.h"
#include "diagnostics/render/render.h"


int driver_run(int argument_count, char *const arguments[],
    FILE *output, FILE *errors) {
    DriverRequest request = driver_parse_request(argument_count, arguments);
    if (request == DRIVER_REQUEST_CHECK || request == DRIVER_REQUEST_STATUS) {
        DriverOptions options;
        driver_options_init(&options, source_system_allocator());
        bool parsed = driver_options_parse(&options, argument_count, arguments);
        int status = EXIT_FAILURE;
        if (parsed) {
            status = request == DRIVER_REQUEST_CHECK ? driver_check(&options, output, errors)
                : driver_status(&options, output, errors);
        } else {
            (void)diagnostic_render(errors, &options.error);
        }
        driver_options_destroy(&options);
        return status;
    }
    FILE *destination = request == DRIVER_REQUEST_HELP ? output : errors;
    int result = driver_write_response(request, destination);

    return request == DRIVER_REQUEST_HELP ? result : EXIT_FAILURE;
}
