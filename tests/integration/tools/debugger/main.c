/** @file
 * @brief Compile and execute the shipped debugger launcher against real retained generated projects.
 * Independent JSON decoding checks names, list occurrences, saved boundaries and source mappings without reading generated C.
 */
#include "../../common.c"
#include <stdbool.h>

/** Evaluate fixture effects before asserting, so disabled assertions cannot remove IO or cleanup. */
static void verify_condition(bool condition, size_t line) {
    if (!condition) { fprintf(stderr, "Debugger fixture check failed at line %zu\n", line); }
    assert(condition);
}
#define require(expression) verify_condition((expression), __LINE__)


/** Build a verified ordinary project first; compile the development client from the same retained assembly and SDK. */
static void build(const char *entry, const char *directory, const char *binary) {
    setup_test();
    char *emit[] = {AMA_EXECUTABLE, "--emit", "project", (char *)entry, "--out", (char *)directory,
        "--sdk", TEST_SDK, "--standard-root", TEST_STDLIB, "--compile", NULL};
    int result = run_compiler_arguments(emit);
    if (result != EXIT_SUCCESS) {
        char text[8192]; read_stream(errors, text, sizeof(text)); fprintf(stderr, "%s", text);
    }
    require(result == EXIT_SUCCESS);
    destroy_test(); setup_test();
    char *compile[] = {TEST_C_COMPILER, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-D_POSIX_C_SOURCE=200809L", "-I", TEST_SDK, "-I", (char *)directory, TEST_LAUNCHER,
        TEST_SDK "/tools/debugger/commands/read.c", TEST_SDK "/tools/debugger/run.c",
        TEST_SDK "/tools/debugger/output/values.c", TEST_SDK "/tools/debugger/output/fields.c",
        TEST_SDK "/tools/debugger/output/cursors.c", TEST_SDK "/tools/debugger/output/instances.c",
        TEST_SDK "/tools/debugger/output/inspect.c", TEST_SDK "/tools/debugger/output/native.c",
        TEST_SDK "/tools/output/json/stream.c", TEST_SDK "/tools/output/json/text.c",
        TEST_SDK "/tools/output/json/values.c", TEST_SDK "/source/unicode/decode.c",
        "-o", (char *)binary, NULL};
    pid_t child = fork(); require(child >= 0);
    if (child == 0) {
        if (dup2(fileno(output), STDOUT_FILENO) < 0 || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(127); }
        execvp(compile[0], compile); perror("Debugger C compiler"); _exit(127);
    }
    int status; require(waitpid(child, &status, 0) == child && WIFEXITED(status));
    if (WEXITSTATUS(status) != 0) {
        char text[16384]; read_stream(errors, text, sizeof(text)); fprintf(stderr, "%s", text);
    }
    require(WEXITSTATUS(status) == 0);
    destroy_test();
}
/** Exact borrowed command bytes enter stdin; native stdout/stderr remain separate from the exclusively created response file. */
static int client(const char *binary, const char *responses, const char *commands) {
    FILE *input = tmpfile(); require(input != NULL);
    require(fwrite(commands, 1, strlen(commands), input) == strlen(commands) && fseek(input, 0, SEEK_SET) == 0);
    pid_t child = fork(); require(child >= 0);
    if (child == 0) {
        if (dup2(fileno(input), STDIN_FILENO) < 0 || dup2(fileno(output), STDOUT_FILENO) < 0
            || dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(127); }
        execl(binary, binary, "--responses", responses, (char *)NULL); _exit(127);
    }
    int status; require(waitpid(child, &status, 0) == child && WIFEXITED(status) && fclose(input) == 0);
    return WEXITSTATUS(status);
}
/** The standard JSON decoder is independent of the C streaming renderer. */
static void verify(const char *mode, const char *responses) {
    pid_t child = fork(); require(child >= 0);
    if (child == 0) { execlp("python3", "python3", "verify.py", mode, responses, (char *)NULL); _exit(127); }
    int status; require(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Repeated inspection is identical until explicit resume advances the saved traversal boundary. */
static void it_inspects_real_yields_original_names_and_terminal_state(void) {
    build(".main", "generated/project", "generated/ama-debugger");
    setup_test();
    require(client("generated/ama-debugger", "generated/accepted.jsonl",
        "run\ninspect\ninspect\nresume\ninspect\nresume\ninspect\nquit\n") == EXIT_SUCCESS);
    verify("accepted", "generated/accepted.jsonl");
    char text[2048]; read_stream(output, text, sizeof(text)); require(text[0] == 0);
    read_stream(errors, text, sizeof(text)); require(text[0] == 0); destroy_test();
}
/** Admission refusals and invalid commands preserve inspectable state; disposal never invents a new implicit root. */
static void it_reports_refusals_invalid_commands_and_disposed_host_access(void) {
    setup_test();
    require(client("generated/ama-debugger", "generated/refused.jsonl",
        "resume\ninspect\nunknown\nrun\nresume\nresume\ninspect\nresume\ninspect\ndispose\ninspect\nrun\nquit\n") == EXIT_FAILURE);
    verify("refused", "generated/refused.jsonl"); destroy_test();
    setup_test();
    require(client("generated/ama-debugger", "generated/empty.jsonl", "") == EXIT_SUCCESS);
    verify("empty", "generated/empty.jsonl"); destroy_test();
    setup_test();
    require(client("generated/ama-debugger", "generated/final-line.jsonl", "run") == EXIT_SUCCESS);
    verify("final", "generated/final-line.jsonl"); destroy_test();
}
/** Existing response bytes cannot be truncated, even when commands would execute a project. */
static void it_refuses_existing_response_files_without_changing_them(void) {
    FILE *file = fopen("generated/existing.jsonl", "wb"); require(file != NULL);
    require(fputs("handwritten response\n", file) != EOF && fclose(file) == 0);
    setup_test(); require(client("generated/ama-debugger", "generated/existing.jsonl", "run\n") == EXIT_FAILURE);
    verify("existing", "generated/existing.jsonl"); destroy_test();
    setup_test(); require(client("generated/ama-debugger", "", "run\n") == EXIT_FAILURE); destroy_test();
    setup_test(); require(client("generated/ama-debugger", "generated/absent/responses", "run\n") == EXIT_FAILURE); destroy_test();
}
/** An accepted native construction failure disposes state; inspection rejects rather than reconstructing dead objects. */
static void it_reports_real_native_failure_and_rejects_dead_state_inspection(void) {
    build(".failed", "generated/failed-project", "generated/failed-debugger");
    setup_test();
    require(client("generated/failed-debugger", "generated/failed.jsonl", "run\ninspect\nquit\n") == EXIT_FAILURE);
    verify("failed", "generated/failed.jsonl");
    char text[8192]; read_stream(errors, text, sizeof(text)); require(text[0] != 0); destroy_test();
}
/** Build inputs stay handwritten and unchanged; generated directories retire only after all real process assertions succeed. */
void run(void) {
    it_inspects_real_yields_original_names_and_terminal_state();
    it_reports_refusals_invalid_commands_and_disposed_host_access();
    it_refuses_existing_response_files_without_changing_them();
    it_reports_real_native_failure_and_rejects_dead_state_inspection();
}
/** Return only after client build, exact decoding, failure and cleanup acceptance. */
int main(void) { run(); return EXIT_SUCCESS; }
