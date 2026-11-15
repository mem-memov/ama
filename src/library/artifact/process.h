/** @file
 * @brief Execute one explicit library tool argument vector without shell evaluation.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) preserves
 * configured tool/path bytes as individual arguments, including spaces.
 */
#ifndef AMA_LIBRARY_ARTIFACT_PROCESS_H
#define AMA_LIBRARY_ARTIFACT_PROCESS_H
#include <stdbool.h>
#include <stdio.h>
/** Wait through interrupted waits; nonzero/signaled children and redirection failures fail. */
bool library_package_process(char *const *, FILE *);
/** As above, capturing stdout into the supplied open stream without shell redirection. */
bool library_package_capture(char *const *, FILE *output, FILE *errors);
#endif
