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

static void setup_test(void) {
    output = tmpfile();
    errors = tmpfile();
    assert(output != NULL);
    assert(errors != NULL);
}

static void destroy_test(void) {
    int output_result = fclose(output);
    int errors_result = fclose(errors);
    assert(output_result == 0);
    assert(errors_result == 0);
}

static int run_compiler(const char *argument) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 ||
            dup2(fileno(errors), STDERR_FILENO) < 0) {
            _exit(127);
        }
        execl(AMA_EXECUTABLE, AMA_EXECUTABLE, argument, (char *) NULL);
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

static void read_stream(FILE *stream, char *buffer, size_t capacity) {
    int seek_result = fseek(stream, 0, SEEK_SET);
    assert(seek_result == 0);
    size_t length = fread(buffer, 1, capacity - 1, stream);
    assert(!ferror(stream));
    assert(length < capacity - 1);
    buffer[length] = '\0';
}

#endif
