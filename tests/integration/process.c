#ifndef AMA_TESTS_INTEGRATION_PROCESS_C
#define AMA_TESTS_INTEGRATION_PROCESS_C
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* argv is passed directly to exec: no shell, interpolation, or command quoting. */
static int run_process(char* const arguments[], const char* output_path, const char* error_path) {
    pid_t child_process = fork();
    assert(child_process >= 0);
    if (child_process == 0) {
        int output_descriptor = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        int error_descriptor = open(error_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (output_descriptor < 0 || error_descriptor < 0) _exit(126);
        if (dup2(output_descriptor, STDOUT_FILENO) < 0 || dup2(error_descriptor, STDERR_FILENO) < 0) _exit(126);
        close(output_descriptor);
        close(error_descriptor);
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t waited_process;
    do { waited_process = waitpid(child_process, &status, 0); } while (waited_process < 0 && errno == EINTR);
    assert(waited_process == child_process);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}
#endif
