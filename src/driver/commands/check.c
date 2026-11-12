/**
 * @file
 * @brief Check the full frontend without native execution or artifact publication.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Report diagnostics while their sources remain alive; successful acknowledgement requires write and flush success.
 */
#include <stdlib.h>
#include "commands.h"
#include "compiler/frontend.h"
#include "diagnostics/render/render.h"

/**
 * Own/check/destroy a frontend; render failure while source spans remain live. Return success
 * only after checked output and flush.
 */
int driver_check(const DriverOptions *options, FILE *output, FILE *errors) {
    CompilerFrontend frontend;
    compiler_frontend_init(&frontend, source_system_allocator());
    bool checked = compiler_frontend_check(&frontend, options->roots, options->root_count,
        options->entry);
    int result = EXIT_FAILURE;
    if (checked) {
        if (fprintf(output, "checked %s\n", options->entry) >= 0 && fflush(output) == 0) {
            result = EXIT_SUCCESS;
        }
    } else {
        (void)diagnostic_render(errors, &frontend.error);
    }
    compiler_frontend_destroy(&frontend);
    return result;
}
