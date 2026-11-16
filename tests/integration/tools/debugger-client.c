/** @file
 * @brief Compile the shipped retained-assembly debugger and execute explicit client command streams for composed tool workflows.
 * Each build owns its process-output fixture; execution borrows the caller's current output/error streams.
 */
#ifndef AMA_TOOL_DEBUGGER_CLIENT_FIXTURE_C
#define AMA_TOOL_DEBUGGER_CLIENT_FIXTURE_C
#include "../common.c"
#include <stdbool.h>
/** Evaluate fixture effects before asserting, preserving IO and cleanup under disabled assertions. */
static void tool_check(bool condition, size_t line) {
    if (!condition) { fprintf(stderr, "Tool debugger fixture failed at line %zu\n", line); }
    assert(condition);
}
#define tool_require(expression) tool_check((expression), __LINE__)
/** Compile one opaque retained assembly with its matching SDK; generated implementation bytes are never parsed. */
static void tool_build_debugger(const char *directory, const char *binary) {
    setup_test();
    char *compile[] = {TEST_C_COMPILER, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-D_POSIX_C_SOURCE=200809L", "-I", TEST_SDK, "-I", (char *)directory, TEST_LAUNCHER,
        TEST_SDK "/tools/debugger/commands/read.c", TEST_SDK "/tools/debugger/run.c",
        TEST_SDK "/tools/debugger/output/values.c", TEST_SDK "/tools/debugger/output/fields.c",
        TEST_SDK "/tools/debugger/output/cursors.c", TEST_SDK "/tools/debugger/output/instances.c",
        TEST_SDK "/tools/debugger/output/inspect.c", TEST_SDK "/tools/debugger/output/native.c",
        TEST_SDK "/tools/output/json/stream.c", TEST_SDK "/tools/output/json/text.c",
        TEST_SDK "/tools/output/json/values.c", TEST_SDK "/source/unicode/decode.c",
        "-o", (char *)binary, NULL};
    pid_t child = fork(); tool_require(child >= 0);
    if (child == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(127); }
        execvp(compile[0], compile); perror("Debugger C compiler"); _exit(127);
    }
    int status; tool_require(waitpid(child, &status, 0) == child && WIFEXITED(status));
    if (WEXITSTATUS(status) != 0) {
        char text[16384]; read_stream(errors, text, sizeof(text)); fprintf(stderr, "%s", text);
    }
    tool_require(WEXITSTATUS(status) == 0);
    destroy_test();
}
/** Feed exact commands while keeping native stdout separate from the exclusively created response file. */
static int tool_run_debugger(const char *binary, const char *responses, const char *commands) {
    FILE *input = tmpfile(); tool_require(input != NULL);
    tool_require(fwrite(commands, 1, strlen(commands), input) == strlen(commands) && fseek(input, 0, SEEK_SET) == 0);
    pid_t child = fork(); tool_require(child >= 0);
    if (child == 0) {
        if (dup2(fileno(input), STDIN_FILENO) < 0 || dup2(fileno(output), STDOUT_FILENO) < 0
            || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(127); }
        execl(binary, binary, "--responses", responses, (char *)NULL); _exit(127);
    }
    int status; tool_require(waitpid(child, &status, 0) == child && WIFEXITED(status) && fclose(input) == 0);
    return WEXITSTATUS(status);
}
#endif
