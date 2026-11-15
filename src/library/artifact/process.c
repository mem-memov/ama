/** @file
 * @brief Keep library packaging subprocess ownership synchronous and shell-free.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires
 * tool failures to prevent successful artifact publication.
 */
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "process.h"
bool library_package_capture(char *const *arguments, FILE *output, FILE *errors) {
    pid_t child = fork();
    if (child < 0) { return false; }
    if (child == 0) {
        if (errors != NULL && dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        if (output != NULL && dup2(fileno(output), STDOUT_FILENO) < 0) { _exit(126); }
        execvp(arguments[0], arguments); _exit(127);
    }
    int status; pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    return waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
bool library_package_process(char *const *arguments, FILE *errors) {
    return library_package_capture(arguments, NULL, errors);
}
