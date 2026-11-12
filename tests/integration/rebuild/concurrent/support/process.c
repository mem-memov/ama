/**
 * @file
 * @brief Own Make subprocesses and the two-pipe manifest comparison protocol.
 *
 * Included only by this leaf's C harness. Children retain their own ready/release
 * ends; the parent controls release and reaps every successful check.
 */
#ifndef AMA_TEST_CONFIGURATION_PROCESS_C
#define AMA_TEST_CONFIGURATION_PROCESS_C

#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

/** One comparator's parent/child pipe ends and its owned process identity. */
typedef struct {
    int ready[2];
    int release[2];
    pid_t process;
} ConfigurationCheck;

/**
 * Publish a child's borrowed descriptor number in its private environment.
 * The descriptor remains open for the shell fixture; formatting/environment
 * failure terminates only the child with status 127.
 */
static void export_descriptor(const char *name, int descriptor) {
    char text[32];
    int length = snprintf(text, sizeof(text), "%d", descriptor);
    if (length <= 0 || (size_t)length >= sizeof(text)) {
        _exit(127);
    }
    int published = setenv(name, text, 1);
    if (published != 0) {
        _exit(127);
    }
}

/** Redirect only the child process, releasing its extra log descriptor. */
static void connect_log(void) {
    int log = open("generated/checks.log", O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (log < 0) {
        _exit(127);
    }
    int output = dup2(log, STDOUT_FILENO);
    int errors = dup2(log, STDERR_FILENO);
    int closed = close(log);
    if (output < 0 || errors < 0 || closed != 0) {
        _exit(127);
    }
}

/** Child Make owns its job scheduling; inherited stale jobserver flags do not. */
static void clear_parent_make_flags(void) {
    int flags = setenv("MAKEFLAGS", "", 1);
    int legacy_flags = setenv("MFLAGS", "", 1);
    if (flags != 0 || legacy_flags != 0) {
        _exit(127);
    }
}

/**
 * Replace the child by Make checking the real configuration target.
 * @param[in] checks Initialized pipe pairs, or NULL for baseline publication.
 * @param[in] selected The child's index, less than two when checks is non-NULL.
 * @param[in] shell_option Borrowed SHELL= argument, or NULL for the default shell.
 * Close all pipe ends this child does not own, redirect its log, and clear stale
 * parent jobserver flags. Setup/exec failure exits 127; success never returns.
 */
static void execute_check(ConfigurationCheck checks[2], size_t selected,
    const char *shell_option) {
    if (checks != NULL) {
        for (size_t index = 0; index < 2; index++) {
            close(checks[index].ready[0]);
            close(checks[index].release[1]);
            if (index != selected) {
                close(checks[index].ready[1]);
                close(checks[index].release[0]);
            }
        }
        export_descriptor("AMA_TEST_CONFIGURATION_READY", checks[selected].ready[1]);
        export_descriptor("AMA_TEST_CONFIGURATION_RELEASE", checks[selected].release[0]);
    }
    connect_log();
    clear_parent_make_flags();
    char *arguments[] = {"make", "--no-print-directory", "-s", "-C",
        "generated/project", "cache/compile.configuration", (char *)shell_option, NULL};
    execvp("make", arguments);
    _exit(127);
}

/**
 * Consume ownership of one positive child PID by waiting for that exact child.
 * Retry interrupted waits; signals/wait failure fail assertions rather than
 * impersonating a normal Make status. Return the child's ordinary exit code.
 */
static int finish_check(pid_t process) {
    assert(process > 0);
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/**
 * Consume/close the parent's ready end after one exact candidate-ready marker.
 * EOF, I/O failure or malformed protocol fails the harness, never releases a
 * recipe under an inferred timing assumption.
 */
static void expect_ready(int descriptor) {
    char byte;
    ssize_t count;
    do { count = read(descriptor, &byte, 1); } while (count < 0 && errno == EINTR);
    assert(count == 1 && byte == 'r');
    int closed = close(descriptor);
    assert(closed == 0);
}

/**
 * Release one ready recipe, close its parent's release end, and reap the child.
 * @param[in,out] check Owns the release descriptor and child PID being consumed.
 * @param[in] index Comparator number used only for failure diagnostics.
 * Protocol/process failure fails the harness and preserves generated evidence.
 */
static void release_check(ConfigurationCheck *check, size_t index) {
    const char byte = 'g';
    ssize_t count;
    do { count = write(check->release[1], &byte, 1); } while (count < 0 && errno == EINTR);
    assert(count == 1);
    int closed = close(check->release[1]);
    assert(closed == 0);
    int status = finish_check(check->process);
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Manifest comparator %zu exited with status %d\n", index, status);
    }
    assert(status == EXIT_SUCCESS);
}

#endif
