/** @file
 * @brief Run exact product commands and compare observable output without reading generated implementations.
 */
#ifndef AMA_ASYNC_EXAMPLES_COMMON
#define AMA_ASYNC_EXAMPLES_COMMON
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/** Preserve exact argument boundaries, capture stdout and require the requested process outcome. */
static void command(char *const *arguments, bool success, const char *expected) {
    int channel[2];
    int opened = pipe(channel);
    assert(opened == 0);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        close(channel[0]);
        if (dup2(channel[1], STDOUT_FILENO) < 0) { _exit(126); }
        close(channel[1]);
        execvp(arguments[0], arguments);
        _exit(127);
    }
    close(channel[1]);
    char output[4096] = {0};
    size_t count = 0;
    ssize_t read_count;
    while ((read_count = read(channel[0], output + count, sizeof(output) - count - 1)) > 0) {
        count += (size_t)read_count;
        assert(count < sizeof(output) - 1);
    }
    assert(read_count == 0);
    close(channel[0]);
    int status = 0;
    pid_t joined = waitpid(child, &status, 0);
    assert(joined == child && WIFEXITED(status));
    assert((WEXITSTATUS(status) == 0) == success);
    if (expected != NULL) {
        if (strcmp(output, expected) != 0) { fprintf(stderr, "Unexpected example output: %s\n", output); }
        assert(strcmp(output, expected) == 0);
    }
}
#endif
