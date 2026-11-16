/**
 * @file
 * @brief Compose checked frontend, project generation, optional verification and publication.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Do not publish an executable before native object verification; output acknowledgement also checks flushing.
 */
#include <stdlib.h>
#include "commands.h"
#include "codegen/codegen.h"
#include "diagnostics/render/render.h"

/**
 * Complete strict frontend, stage project artifacts and optionally compile/verify before
 * publication. Destroy scaffold/frontend on all paths; acknowledged success requires stream
 * flush.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04). */
int driver_emit(const DriverOptions *options, FILE *output, FILE *errors) {
    CompilerFrontend frontend;
    compiler_frontend_init(&frontend, source_system_allocator());
    int result = EXIT_FAILURE;
    bool checked = compiler_frontend_check(&frontend, options->roots, options->root_count, options->entry);
    bool warnings = driver_warnings(&frontend.project, errors);
    if (!checked) {
        (void)diagnostic_render(errors, &frontend.error);
    } else if (warnings) {
        NativeScaffold assembly;
        native_scaffold_init(&assembly, &frontend, source_system_allocator());
        ScaffoldOptions generation = {.directory = options->output, .sdk = options->sdk, .compiler = options->compiler,
            .flags = options->c_flags, .flag_count = options->c_flag_count, .overwrite = options->overwrite};
        if (project_codegen_build(&assembly, generation) && (!options->compile || project_codegen_compile(&assembly, errors)) && native_scaffold_publish(&assembly)) {
            if (fprintf(output, "emitted project assembly %s\n", options->entry) >= 0 && fflush(output) == 0) { result = EXIT_SUCCESS; }
        } else { (void)diagnostic_render(errors, &assembly.error); }
        native_scaffold_destroy(&assembly);
    }
    compiler_frontend_destroy(&frontend);
    return result;
}
