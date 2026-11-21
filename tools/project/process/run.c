/** @file
 * @brief Collect exact process launch, exit and signal results without evaluating shell text.
 * [PROJECT-03](../../../spec/implementation/project-tools.md#project-03)
 * requires stage-specific observed outcomes and isolated synchronous execution.
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "process.h"
/** A private close-on-exec pipe distinguishes exec failure from a program that
 * legitimately exits 127. Only async-signal-safe operations run after fork. */
static void child(char *const *arguments, const char *directory, int output, int errors, int report) {
    if ((!directory || chdir(directory) == 0) &&
        (output < 0 || dup2(output, STDOUT_FILENO) >= 0) &&
        (errors < 0 || dup2(errors, STDERR_FILENO) >= 0)) { execvp(arguments[0], arguments); }
    int failure = errno;
    const unsigned char *bytes = (const unsigned char *)&failure;
    size_t remaining = sizeof(failure);
    while (remaining) {
        ssize_t written = write(report, bytes, remaining);
        if (written < 0 && errno == EINTR) { continue; }
        if (written <= 0) { break; }
        bytes += written; remaining -= (size_t)written;
    }
    _exit(127);
}
/** EOF before any report byte denotes successful exec; partial reports refuse. */
static bool launch_error(int descriptor, int *error) {
    unsigned char *bytes = (unsigned char *)error;
    size_t count = 0;
    while (count < sizeof(*error)) {
        ssize_t received = read(descriptor, bytes + count, sizeof(*error) - count);
        if (received < 0 && errno == EINTR) { continue; }
        if (received < 0) { return false; }
        if (!received) { return count == 0; }
        count += (size_t)received;
    }
    return true;
}
bool project_process(char *const *arguments, const char *directory, FILE *output, FILE *errors, ProjectProcessResult *result) {
    if (!arguments || !arguments[0] || !*arguments[0] || !result ||
        (output && fflush(output)) || (errors && fflush(errors))) { return false; }
    int output_descriptor = output ? fileno(output) : -1;
    int error_descriptor = errors ? fileno(errors) : -1;
    if ((output && output_descriptor < 0) || (errors && error_descriptor < 0)) { return false; }
    int descriptors[2];
    if (pipe(descriptors) != 0) { return false; }
    if (fcntl(descriptors[1], F_SETFD, FD_CLOEXEC) != 0) { (void)close(descriptors[0]); (void)close(descriptors[1]); return false; }
    pid_t process = fork();
    if (process < 0) { (void)close(descriptors[0]); (void)close(descriptors[1]); return false; }
    if (!process) { (void)close(descriptors[0]); child(arguments, directory, output_descriptor, error_descriptor, descriptors[1]); }
    (void)close(descriptors[1]);
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    ProjectProcessResult observed = {0};
    bool complete = launch_error(descriptors[0], &observed.error);
    (void)close(descriptors[0]);
    if (!complete || waited != process || (!WIFEXITED(status) && !WIFSIGNALED(status))) { return false; }
    if (WIFEXITED(status)) { observed.exit_code = WEXITSTATUS(status); }
    else { observed.signal = WTERMSIG(status); }
    *result = observed;
    return true;
}
