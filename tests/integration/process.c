#ifndef AMA_TESTS_INTEGRATION_PROCESS_C
#define AMA_TESTS_INTEGRATION_PROCESS_C
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* argv is passed directly to exec: no shell, interpolation, or command quoting. */
static int run_process(char* const argv[], const char* output, const char* errors) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        int out = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        int err = open(errors, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (out < 0 || err < 0) _exit(126);
        if (dup2(out, STDOUT_FILENO) < 0 || dup2(err, STDERR_FILENO) < 0) _exit(126);
        close(out);
        close(err);
        execvp(argv[0], argv);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}
#endif
