/** @file
 * @brief Own real CLI compilation and captured execution for the maintained constructor project.
 */
#ifndef AMA_OBJECT_INTEGRATION_COMMON
#define AMA_OBJECT_INTEGRATION_COMMON
#include <stdbool.h>
#include "../common.c"

/** Execute only a published program with separate streams and optional target
 * sanitizer policy. Do not inspect generated sources, object files or binaries.
 */
static int run_object_program(const char *program, bool sanitized) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        if (sanitized && (setenv("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1", 1) != 0
            || setenv("UBSAN_OPTIONS", "halt_on_error=1", 1) != 0)) { _exit(126); }
        execl(program, program, (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/** Verify and publish one exact entry, then compare its runtime streams/status.
 * Compiler messages cannot become program output. Expected failures are still
 * verified executables; source/frontend/backend failures use separate checks.
 */
static void compile_and_run_object(const char *entry, const char *expected,
    const char *expected_error, bool sanitized) {
    setup_test();
    char directory[128], executable[160];
    int length = snprintf(directory, sizeof(directory), "generated/%s/%s", sanitized ? "sanitized" : "ordinary", entry + 1);
    assert(length > 0 && (size_t)length < sizeof(directory));
    length = snprintf(executable, sizeof(executable), "%s/program", directory);
    assert(length > 0 && (size_t)length < sizeof(executable));
    char *ordinary[] = {AMA_EXECUTABLE, "--emit", "project", (char *)entry, "--out", directory, "--sdk", TEST_SDK, "--compile", NULL};
    char *instrumented[] = {AMA_EXECUTABLE, "--emit", "project", (char *)entry, "--out", directory, "--sdk", TEST_SDK, "--compile",
        "--c-flag", "-fsanitize=address,undefined", "--c-flag", "-fno-omit-frame-pointer", "--c-flag", "-fno-pie", "--c-flag", "-no-pie", NULL};
    int result = run_compiler_arguments(sanitized ? instrumented : ordinary);
    char diagnostic[8192];
    read_stream(errors, diagnostic, sizeof(diagnostic));
    if (result != EXIT_SUCCESS) { fprintf(stderr, "Object compilation failed for %s: %s\n", entry, diagnostic); }
    assert(result == EXIT_SUCCESS && diagnostic[0] == '\0');
    destroy_test();
    setup_test();
    result = run_object_program(executable, sanitized);
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert((result == EXIT_SUCCESS) == (expected_error == NULL));
    if (expected_error == NULL) { assert(diagnostic[0] == '\0'); }
    else { assert(strstr(diagnostic, expected_error) != NULL); }
    char actual[4096];
    read_stream(output, actual, sizeof(actual));
    if (strcmp(actual, expected) != 0) { fprintf(stderr, "Unexpected object effects:\n%s\n", actual); }
    assert(strcmp(actual, expected) == 0);
    destroy_test();
}
#endif
