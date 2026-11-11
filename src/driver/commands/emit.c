#include <stdlib.h>
#include "commands.h"
#include "codegen/codegen.h"
#include "diagnostics/render/render.h"

int driver_emit(const DriverOptions *options, FILE *output, FILE *errors) {
    CompilerFrontend frontend;
    compiler_frontend_init(&frontend, source_system_allocator());
    int result = EXIT_FAILURE;
    if (!compiler_frontend_check(&frontend, options->roots, options->root_count, options->entry, options->libraries)) {
        (void)diagnostic_render(errors, &frontend.error);
    } else {
        NativeScaffold assembly;
        native_scaffold_init(&assembly, &frontend, source_system_allocator());
        ScaffoldOptions generation = {.directory = options->output, .sdk = options->sdk, .compiler = options->compiler,
            .flags = options->c_flags, .flag_count = options->c_flag_count, .overwrite = options->overwrite};
        if (project_codegen_build(&assembly, generation) && native_scaffold_publish(&assembly)) {
            if (fprintf(output, "emitted project assembly %s\n", options->entry) >= 0 && fflush(output) == 0) { result = EXIT_SUCCESS; }
        } else { (void)diagnostic_render(errors, &assembly.error); }
        native_scaffold_destroy(&assembly);
    }
    compiler_frontend_destroy(&frontend);
    return result;
}
