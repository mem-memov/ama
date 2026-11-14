/** @file
 * @brief Check an explicit component and write resolved metadata or its public C header.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) separates
 * export validation from executable launchers and later library packaging.
 */
#include <stdlib.h>
#include "commands.h"
#include "library/model.h"
#include "diagnostics/render/render.h"
int driver_library(const DriverOptions *options, FILE *output, FILE *errors) {
    LibraryBuild build;
    library_build_init(&build, source_system_allocator());
    bool valid = library_build_check(&build, options->roots, options->root_count, options->library_configuration);
    int result = EXIT_FAILURE;
    if (!valid) { (void)diagnostic_render(errors, &build.error); }
    else if (options->request == DRIVER_REQUEST_LIBRARY_HEADER && !library_header_target(&build, &build.error)) {
        (void)diagnostic_render(errors, &build.error);
    } else if (driver_warnings(&build.frontend.project, errors)) {
        bool written = options->request == DRIVER_REQUEST_LIBRARY_HEADER
            ? library_header_write(&build, output) : library_manifest_write(&build, output);
        if (written) { result = EXIT_SUCCESS; }
    }
    library_build_destroy(&build);
    return result;
}
