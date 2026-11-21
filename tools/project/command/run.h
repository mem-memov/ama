/** @file
 * @brief Expose the separate project command adapter with explicit output ownership.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * forbids depending on the compiler's mutable dispatcher.
 */
#ifndef AMA_PROJECT_COMMAND_RUN_H
#define AMA_PROJECT_COMMAND_RUN_H
#include <stdio.h>
/** Borrow argv and streams through completion; 0 success, 2 usage, 1 operation failure. */
int project_command_run(int, const char *const *, FILE *, FILE *);
#endif
