#include <stdlib.h>
#include "commands.h"
#include "compiler/inspect/module.h"
#include "diagnostics/render/render.h"

int driver_status(const DriverOptions *options, FILE *output, FILE *errors) {
    ModuleInspection inspection;
    compiler_inspection_init(&inspection, source_system_allocator());
    bool inspected = compiler_inspect_module(&inspection, options->roots, options->root_count, options->entry);
    int status = EXIT_FAILURE;
    if (inspected) {
        const char *const kinds[] = {"draft", "constants", "ama", "native"};
        if (fprintf(output, "%s %s\n", kinds[inspection.syntax.sections.kind], options->entry) >= 0
            && fflush(output) == 0) {
            status = EXIT_SUCCESS;
        }
    } else {
        (void)diagnostic_render(errors, &inspection.error);
    }
    compiler_inspection_destroy(&inspection);
    return status;
}
