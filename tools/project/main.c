/** @file
 * @brief Adapt process arguments to the independently linked project-tool dispatcher.
 * [PROJECT-01](../../spec/implementation/project-tools.md#project-01)
 * keeps project effects outside compiler command ownership.
 */
#include "command/run.h"
/** Borrow standard streams and argv for one synchronous project request. */
int main(int count, char **arguments) {
    return project_command_run(count, (const char *const *)arguments, stdout, stderr);
}
