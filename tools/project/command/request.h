/** @file
 * @brief Recognize project requests without compilation or filesystem effects.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * assigns command parsing to the independent project executable.
 */
#ifndef AMA_PROJECT_COMMAND_REQUEST_H
#define AMA_PROJECT_COMMAND_REQUEST_H
#include <stdbool.h>
/** Immutable borrowed request; argv storage must outlive execution. */
typedef struct { const char *destination; } ProjectInitRequest;
/** Accept only `init DESTINATION`; failure leaves output unchanged. No allocation. */
bool project_command_request(int count, const char *const *arguments, ProjectInitRequest *output);
#endif
