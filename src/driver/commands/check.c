#include <stdlib.h>
#include "commands.h"
#include "diagnostics/render/render.h"

int driver_check(const DriverOptions *options, FILE *output, FILE *errors) {
    CompilerFrontend frontend;
    compiler_frontend_init(&frontend, source_system_allocator());
    bool checked = compiler_frontend_check(&frontend, options->roots, options->root_count,
        options->entry, options->libraries);
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
