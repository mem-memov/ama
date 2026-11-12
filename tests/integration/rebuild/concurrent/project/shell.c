/**
 * @file
 * @brief Pause manifest recipe shells after Make writes their candidate files.
 *
 * Two checked pipes let the parent choose an exact execution order without
 * relying on a sleep or scheduler luck. Other shell requests execute normally.
 * It exercises recursive Make scheduling and command-manifest candidate isolation.
 */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Read one inherited descriptor without duplicating it or changing its owner.
 * @param[in] name Borrowed environment-variable name selecting a pipe end.
 * @return The descriptor number, or -1 for absent/malformed/out-of-range text.
 */
static int descriptor(const char *name) {
    const char *text = getenv(name);
    if (text == NULL || *text == 0) {
        return -1;
    }
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    return errno == 0 && *end == 0 && value >= 0 && value <= INT_MAX
        ? (int)value : -1;
}

/**
 * Publish candidate readiness and consume exactly one parent release marker.
 * Both inherited pipe ends are closed on success. Failure returns -1 and the
 * caller exits, letting process teardown close any remaining ends.
 * @return Zero after permission, or -1 on controller/I/O failure.
 */
static int await_release(void) {
    int ready = descriptor("AMA_TEST_CONFIGURATION_READY");
    int release = descriptor("AMA_TEST_CONFIGURATION_RELEASE");
    if (ready < 0 || release < 0) {
        return -1;
    }
    char byte = 'r';
    ssize_t result;
    do { result = write(ready, &byte, 1); } while (result < 0 && errno == EINTR);
    int published = close(ready);
    if (result != 1 || published != 0) {
        return -1;
    }
    do { result = read(release, &byte, 1); } while (result < 0 && errno == EINTR);
    int closed = close(release);
    return result == 1 && byte == 'g' && closed == 0 ? 0 : -1;
}

/**
 * Preserve ordinary /bin/sh behavior after the optional manifest barrier.
 * Borrow argv until exec, replacing only argv[0]. The fixture recognizes the
 * manifest recipe's fixed comparison command solely to select its test barrier;
 * Make's other shell requests pass through without waiting.
 * @return 127 on barrier/exec failure; successful exec has the real shell status.
 */
int main(int argument_count, char **arguments) {
    if (argument_count == 3 && strcmp(arguments[1], "-c") == 0
        && strstr(arguments[2], "cmp -s") != NULL && await_release() != 0) {
        return 127;
    }
    arguments[0] = "sh";
    execv("/bin/sh", arguments);
    return 127;
}
