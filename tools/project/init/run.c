/** @file
 * @brief Orchestrate one explicit scaffold transaction without compiler state.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * isolates project effects; [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * requires cleanup after every incomplete preparation.
 */
#include "internal.h"
bool project_init_fail(ProjectInitState *state, ProjectInitStatus status, int error, const char *message) {
    if (state->result.status == PROJECT_INIT_OK) {
        state->result.status = status;
        state->result.system_error = error;
        state->result.message = message;
    }
    return false;
}
bool project_init_permit(ProjectInitState *state, ProjectInitOperation operation, const char *path) {
    return !state->control.permit || state->control.permit(state->control.context, operation, path) ||
        project_init_fail(state, PROJECT_INIT_CANCELLED, 0, "Project initialization was cancelled before an operation");
}
ProjectInitResult project_init(const char *destination, SourceAllocator allocator, ProjectInitControl control) {
    ProjectInitState state = {.allocator = allocator, .control = control,
        .parent = -1, .stage = -1, .packages = -1};
    if (!destination || !*destination || !allocator.allocate || !allocator.release) {
        state.result = (ProjectInitResult){.status = PROJECT_INIT_INVALID, .message = "An explicit destination and allocation authority are required"};
        return state.result;
    }
    if (project_init_parent(&state, destination) && project_init_preflight(&state) &&
        project_init_prepare(&state)) { (void)project_init_publish(&state); }
    project_init_cleanup(&state);
    return state.result;
}
