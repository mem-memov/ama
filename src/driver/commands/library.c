/** @file
 * @brief Check an explicit C-library configuration and report its resolved ABI metadata.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) separates
 * export validation from executable launchers and later library packaging.
 */
#include <stdlib.h>
#include "commands.h"
#include "library/model.h"
#include "diagnostics/render/render.h"
int driver_library_check(const DriverOptions *options, FILE *output, FILE *errors) {
    LibraryBuild build;
    library_build_init(&build, source_system_allocator());
    bool valid = library_build_check(&build, options->roots, options->root_count, options->library_configuration);
    int result = EXIT_FAILURE;
    if (!valid) { (void)diagnostic_render(errors, &build.error); }
    else if (driver_warnings(&build.frontend.project, errors) && library_manifest_write(&build, output)) { result = EXIT_SUCCESS; }
    library_build_destroy(&build);
    return result;
}
