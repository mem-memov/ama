/**
 * @file
 * @brief Run controlled Make scenarios and observe their exact exit/output behavior.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_HARNESS_PROCESS_C
#define AMA_TEST_HARNESS_PROCESS_C

/**
 * Child-only adapter: capture Make output, disable core files, clear inherited job flags and
 * exec exact argv. Setup/exec failure exits distinctly.
 */
static void execute_make(char *const arguments[]) {
    int descriptor = open("generated/make.log", O_WRONLY | O_CREAT | O_APPEND, 0600);
    struct rlimit limit = {0, 0};
    if (descriptor < 0 || dup2(descriptor, STDOUT_FILENO) < 0 ||
        dup2(descriptor, STDERR_FILENO) < 0 || setrlimit(RLIMIT_CORE, &limit) != 0 ||
        setenv("MAKEFLAGS", "", 1) != 0 || setenv("MFLAGS", "", 1) != 0) {
        _exit(127);
    }
    close(descriptor);
    execvp("make", arguments);
    _exit(127);
}

/**
 * Run one isolated Make scenario with explicit compiler/flags and return its normal exit code.
 * Assertions verify fork/wait; no product command is inferred.
 */
static int run_make(const char *directory, const char *target, const char *override) {
    char *arguments[] = {
        "make", "-s", "-j4", "-C", (char *) directory, (char *) target,
        "C_COMPILER=" TEST_REAL_COMPILER,
        "C_COMPILER_FLAGS=-O0 -std=c11 -DNDEBUG", "CPPFLAGS=", "LTO_FLAGS=",
        "LINKER_FLAGS=", "TEST_DEFINES=", (char *) override, NULL
    };
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execute_make(arguments);
    }
    int status;
    pid_t waited;
    do {
        waited = waitpid(process, &status, 0);
    } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

#endif
