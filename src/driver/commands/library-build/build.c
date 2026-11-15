/** @file
 * @brief Orchestrate checked closure generation, ELF packaging and safe publication.
 * [EMBED-06](../../../../spec/implementation/c-library.md#embed-06) requires
 * complete target-verified artifacts before a C host consumes the component.
 */
#include <stdlib.h>
#include "internal.h"
#include "codegen/codegen.h"
#include "library/artifact/package.h"
#include "diagnostics/render/render.h"
/** Package only private staged files and retain them before removing scratch. */
static bool package(NativeScaffold *scaffold, const LibraryBuild *build, FILE *errors) {
    char stage[]="/tmp/ama-library-XXXXXX";
    if (mkdtemp(stage)==NULL) { return false; }
    LibraryPackageOptions options={.assembly=scaffold,.compiler=scaffold->options.compiler,
        .archiver="ar",.object_copy="objcopy",.sdk=scaffold->sdk,.includes=stage,
        .source=scaffold_join(scaffold,stage,"/","component.c"),
        .raw_object=scaffold_join(scaffold,stage,"/","raw.o"),
        .isolated_object=scaffold_join(scaffold,stage,"/","isolated.o"),
        .symbols=scaffold_join(scaffold,stage,"/","symbols.txt"),
        .archive=scaffold_join(scaffold,stage,"/","library.a"),
        .shared=scaffold_join(scaffold,stage,"/","library.so")};
    bool built=driver_library_stage(scaffold,build,stage)
        && library_package_build(build,&options,errors) && driver_library_retain(scaffold,stage);
    driver_library_cleanup(stage);
    return built;
}
bool driver_library_build(const DriverOptions *options, const LibraryBuild *build, FILE *errors) {
    NativeScaffold scaffold;
    native_scaffold_init(&scaffold,&build->frontend,source_system_allocator());
    bool built=project_codegen_build(&scaffold,(ScaffoldOptions){.directory=options->output,
        .sdk=options->sdk,.compiler=options->compiler,.overwrite=options->overwrite});
    if (built) { built=package(&scaffold,build,errors); }
    if (built) { built=native_scaffold_publish(&scaffold); }
    if (!built) {
        if (scaffold.error.message==NULL) {
            (void)scaffold_error(&scaffold,DIAGNOSTIC_IO_FAILURE,"Cannot finish the verified C-library artifacts",NULL);
        }
        (void)diagnostic_render(errors,&scaffold.error);
    }
    native_scaffold_destroy(&scaffold);
    return built;
}
