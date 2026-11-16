/**
 * @file
 * @brief Report source activation kind using syntax-only inspection.
 *
 * [SYN-04](../../../spec/language/source-and-names.md#syn-04)
 * Draft status is informative inspection, never executable semantic acceptance.
 */
#include <stdlib.h>
#include "commands.h"
#include "compiler/inspect/module.h"
#include "diagnostics/render/render.h"

/**
 * Own syntax-only inspection, report its activation kind and flush output. Destroy inspection
 * after rendering source-backed diagnostics.
 * Component contract: [SYN-04](../../../spec/language/source-and-names.md#syn-04). */
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
