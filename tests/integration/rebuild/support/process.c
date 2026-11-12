/**
 * @file
 * @brief Execute checked build subprocesses and observe resulting test programs.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_BUILD_PROCESS_C
#define AMA_TEST_BUILD_PROCESS_C

#include <fcntl.h>

/**
 * Wait for the exact child through EINTR, assert normal exit and return its actual code; signals
 * are not silently treated as expected compile failure.
 */
static int wait_for_process(pid_t process) {
    assert(process >= 0);
    int status;
    pid_t waited;
    do {
        waited = waitpid(process, &status, 0);
    } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/**
 * Child-only setup: append build logs, clear inherited Make flags, install exact compiler/trace
 * variables and exec Make; failure exits 127.
 */
static void execute_build(char *const arguments[]) {
    int descriptor = open("generated/make.log", O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (descriptor < 0 || dup2(descriptor, STDOUT_FILENO) < 0 ||
        dup2(descriptor, STDERR_FILENO) < 0) {
        _exit(127);
    }
    close(descriptor);
    if (setenv("MAKEFLAGS", "", 1) != 0 || setenv("MFLAGS", "", 1) != 0 ||
        setenv("AMA_TEST_REAL_COMPILER", TEST_REAL_COMPILER, 1) != 0 ||
        setenv("AMA_TEST_TRACE", trace_path, 1) != 0) {
        _exit(127);
    }
    execvp("make", arguments);
    _exit(127);
}

/**
 * Compose exact Make arguments for one isolated target/override, run the child and require
 * successful completion; recorded invocations remain observable separately.
 */
static void run_build(const char *directory, const char *target, const char *override) {
    char compiler_option[4096];
    int length = snprintf(compiler_option, sizeof(compiler_option),
                            "C_COMPILER=%s/generated/compiler", working_directory);
    assert(length > 0 && (size_t) length < sizeof(compiler_option));
    char *arguments[] = {
        "make", "-s", "-j4", "-C", (char *) directory, (char *) target,
        compiler_option, "C_COMPILER_FLAGS=-O0 -std=c11", "CPPFLAGS=",
        "LTO_FLAGS=", "LINKER_FLAGS=", "TEST_DEFINES=", (char *) override, NULL
    };
    pid_t process = fork();
    if (process == 0) {
        execute_build(arguments);
    }
    int result = wait_for_process(process);
    assert(result == EXIT_SUCCESS);
}

/**
 * Run the isolated built program and require its exact normal exit value, proving the changed
 * dependency affected linked behavior.
 */
static void expect_program_value(int expected) {
    pid_t process = fork();
    if (process == 0) {
        execl("generated/project/ama", "ama", (char *) NULL);
        _exit(127);
    }
    int result = wait_for_process(process);
    assert(result == expected);
}

#endif
