/**
 * @file
 * @brief Own isolated compiler-process output/error streams and checked child execution.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_INTEGRATION_COMMON_C
#define AMA_TEST_INTEGRATION_COMMON_C

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static FILE *output;
static FILE *errors;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    output = tmpfile();
    errors = tmpfile();
    assert(output != NULL);
    assert(errors != NULL);
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    int output_result = fclose(output);
    int errors_result = fclose(errors);
    assert(output_result == 0);
    assert(errors_result == 0);
}

/**
 * Fork the actual compiler with exact borrowed argv, route its output into fixture streams and
 * wait through EINTR. Require normal exit and return its actual status; never use a shell.
 */
static int run_compiler_arguments(char *const arguments[]) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 ||
            dup2(fileno(errors), STDERR_FILENO) < 0) {
            _exit(127);
        }
        execv(AMA_EXECUTABLE, arguments);
        _exit(127);
    }
    int status = 0;
    pid_t waited;
    do {
        waited = waitpid(process, &status, 0);
    } while (waited < 0 && errno == EINTR);
    assert(waited == process);
    assert(WIFEXITED(status));
    return WEXITSTATUS(status);
}

/**
 * Run one explicit compiler argument through the same checked process adapter; retain no
 * argument pointer after child completion.
 */
static inline int run_compiler(const char *argument) {
    char *arguments[] = {AMA_EXECUTABLE, (char *)argument, NULL};
    return run_compiler_arguments(arguments);
}

/**
 * Rewind a borrowed stream and read a bounded NUL-terminated observation. Require spare capacity
 * and successful IO; the caller owns the buffer and stream.
 */
static void read_stream(FILE *stream, char *buffer, size_t capacity) {
    int seek_result = fseek(stream, 0, SEEK_SET);
    assert(seek_result == 0);
    size_t length = fread(buffer, 1, capacity - 1, stream);
    assert(!ferror(stream));
    assert(length < capacity - 1);
    buffer[length] = '\0';
}

#endif
