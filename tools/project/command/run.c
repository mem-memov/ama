/** @file
 * @brief Dispatch explicit initialization and report primary/cleanup failures separately.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * requires useful conflict and retained-staging diagnostics without implicit repairs.
 */
#include <string.h>
#include "request.h"
#include "run.h"
#include "../init/init.h"
int project_command_run(int count, const char *const *arguments, FILE *output, FILE *errors) {
    ProjectInitRequest request;
    if (!project_command_request(count, arguments, &request)) {
        fputs("Usage: ama-project init DESTINATION\n", errors);
        return 2;
    }
    SourceAllocator allocator = source_system_allocator();
    ProjectInitResult result = project_init(request.destination, allocator, (ProjectInitControl){0});
    if (result.status != PROJECT_INIT_OK) {
        fprintf(errors, "ama-project: %s: %s", request.destination, result.message);
        if (result.system_error) { fprintf(errors, ": %s", strerror(result.system_error)); }
        fputc('\n', errors);
        if (result.cleanup_failed) { fprintf(errors, "Private staging retained for recovery: %s\n", result.retained_stage); }
    } else { fprintf(output, "Created empty Ama project: %s\n", request.destination); }
    allocator.release(allocator.context, result.retained_stage);
    return result.status == PROJECT_INIT_OK ? 0 : 1;
}
