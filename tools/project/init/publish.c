/** @file
 * @brief Publish the complete prepared project at one kernel rename boundary.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * permits only a new or empty ordinary destination, never a merge.
 */
#include <errno.h>
#include <stdio.h>
#include "internal.h"
bool project_init_publish(ProjectInitState *state) {
    if (!project_init_permit(state, PROJECT_INIT_PUBLISH, state->name) || !project_init_preflight(state)) { return false; }
    if (renameat(state->parent, state->stage_name, state->parent, state->name) != 0) {
        return project_init_fail(state, PROJECT_INIT_IO, errno, "Atomic project publication failed; the destination was not merged");
    }
    state->staged = false;
    return true;
}
