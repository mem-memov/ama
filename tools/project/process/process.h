/** @file
 * @brief Own synchronous shell-free processes for explicit project operations.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * keeps process orchestration outside the compiler dispatcher.
 */
#ifndef AMA_PROJECT_PROCESS_H
#define AMA_PROJECT_PROCESS_H
#include <stdbool.h>
#include <stdio.h>
/** Exact child observation: nonzero error means exec/redirection/chdir failed,
 * signal means signal termination, otherwise exit_code is the normal status. */
typedef struct { int exit_code, signal, error; } ProjectProcessResult;
/** Borrow NULL-terminated argv, optional working directory and optional open
 * capture streams synchronously. Flush streams before forking. No shell is used.
 * Return false on invalid input/spawn/wait/protocol failure; true also for an
 * observed unsuccessful child. output is assigned only after collection. */
bool project_process(char *const *, const char *, FILE *, FILE *, ProjectProcessResult *);
#endif
