#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int record_invocation(int argument_count, char **arguments) {
    const char *kind = "link";
    const char *source = "-";
    for (int index = 1; index + 1 < argument_count; index++) {
        if (strcmp(arguments[index], "-c") == 0) {
            kind = "compile";
            source = arguments[index + 1];
            break;
        }
    }
    char record[4096];
    int length = snprintf(record, sizeof(record), "%s %s\n", kind, source);
    if (length < 0 || (size_t) length >= sizeof(record)) {
        return -1;
    }
    int descriptor = open(getenv("AMA_TEST_TRACE"), O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (descriptor < 0) {
        return -1;
    }
    ssize_t written = write(descriptor, record, (size_t) length);
    int result = close(descriptor);
    return written == length && result == 0 ? 0 : -1;
}

int main(int argument_count, char **arguments) {
    const char *compiler = getenv("AMA_TEST_REAL_COMPILER");
    if (compiler == NULL || getenv("AMA_TEST_TRACE") == NULL ||
        record_invocation(argument_count, arguments) != 0) {
        return 127;
    }
    arguments[0] = (char *) compiler;
    execvp(compiler, arguments);
    return 127;
}
