/**
 * @file
 * @brief Dispatch one explicit command after exact option validation.
 *
 * [COMP-01](../../spec/implementation/compiler.md#comp-01)
 * Options own their arrays; frontend/scaffold command owners are destroyed before returning. Help and unavailable requests bypass operand parsing.
 */
#include "driver.h"

#include <stdlib.h>

#include "output/output.h"
#include "request/request.h"
#include "commands/commands.h"
#include "commands/editor/editor.h"
#include "commands/relocation/relocation.h"
#include "diagnostics/render/render.h"


/**
 * Recognize one exact request, validate owned option arrays, invoke one command and destroy
 * options on every return. Borrow argv/streams without modification or closing.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
int driver_run(int argument_count, char *const arguments[],
    FILE *output, FILE *errors) {
    DriverRequest request = driver_parse_request(argument_count, arguments);
    if (request == DRIVER_REQUEST_SOURCE_DOMAIN || request == DRIVER_REQUEST_CHECK || request == DRIVER_REQUEST_STATUS
        || request == DRIVER_REQUEST_NATIVE || request == DRIVER_REQUEST_EMIT
        || request == DRIVER_REQUEST_LIBRARY_CHECK || request == DRIVER_REQUEST_LIBRARY_HEADER
        || request == DRIVER_REQUEST_LIBRARY_BUILD || request == DRIVER_REQUEST_EDITOR
        || request == DRIVER_REQUEST_RENAME_PLAN || request == DRIVER_REQUEST_RELOCATE_PREVIEW
        || request == DRIVER_REQUEST_RELOCATE_APPLY || request == DRIVER_REQUEST_RELOCATE_RECOVER) {
        DriverOptions options;
        driver_options_init(&options, source_system_allocator());
        bool parsed = driver_options_parse(&options, argument_count, arguments);
        int status = EXIT_FAILURE;
        if (parsed) {
            status = request == DRIVER_REQUEST_SOURCE_DOMAIN ? driver_source_domain(&options, output, errors)
                : request == DRIVER_REQUEST_CHECK ? driver_check(&options, output, errors)
                : request == DRIVER_REQUEST_STATUS ? driver_status(&options, output, errors)
                : request == DRIVER_REQUEST_NATIVE ? driver_native(&options, output, errors)
                : request == DRIVER_REQUEST_EMIT ? driver_emit(&options, output, errors)
                : request == DRIVER_REQUEST_EDITOR || request == DRIVER_REQUEST_RENAME_PLAN
                    ? driver_editor(&options,output,errors)
                : request == DRIVER_REQUEST_RELOCATE_PREVIEW || request == DRIVER_REQUEST_RELOCATE_APPLY
                    || request == DRIVER_REQUEST_RELOCATE_RECOVER ? driver_relocation(&options,output,errors)
                : driver_library(&options, output, errors);
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
