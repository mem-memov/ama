/** @file
 * @brief Validate the complete initialization command before any side effect.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * requires an explicit destination and forbids implicit project creation.
 */
#include <string.h>
#include "request.h"
bool project_command_request(int count, const char *const *arguments, ProjectInitRequest *output) {
    if (count != 3 || !arguments || !output || !arguments[0] ||
        !arguments[1] || strcmp(arguments[1], "init") || !arguments[2] || !arguments[2][0]) { return false; }
    *output = (ProjectInitRequest){.destination = arguments[2]};
    return true;
}
