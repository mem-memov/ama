/** @file
 * @brief Compile and execute the shipped debugger launcher against real retained generated projects.
 * Independent JSON decoding checks names, list occurrences, saved boundaries and source mappings without reading generated C.
 */
#include "../debugger-client.c"
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
        "--sdk", TEST_SDK, "--standard-root", TEST_STDLIB, "--compile", "--mode", "executable", "--component", "test.application", NULL};
    int result = run_compiler_arguments(emit);
    if (result != EXIT_SUCCESS) {
        char text[8192]; read_stream(errors, text, sizeof(text)); fprintf(stderr, "%s", text);
    }
    require(result == EXIT_SUCCESS);
    destroy_test();
    tool_build_debugger(directory, binary);
}
/** The standard JSON decoder is independent of the C streaming renderer. */
static void verify(const char *mode, const char *responses) {
    pid_t child = fork(); require(child >= 0);
    if (child == 0) { execlp("python3", "python3", "verify.py", mode, responses, (char *)NULL); _exit(127); }
    int status; require(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Repeated inspection is identical until explicit resume advances the saved traversal boundary. */
static void it_inspects_real_yields_original_names_and_terminal_state(void) {
    build("/test/fixture/v1.0.0/src/main", "generated/project", "generated/ama-debugger");
    setup_test();
    require(tool_run_debugger("generated/ama-debugger", "generated/accepted.jsonl",
        "run\ninspect\ninspect\nresume\ninspect\nresume\ninspect\nquit\n") == EXIT_SUCCESS);
    verify("accepted", "generated/accepted.jsonl");
    char text[2048]; read_stream(output, text, sizeof(text)); require(text[0] == 0);
    read_stream(errors, text, sizeof(text)); require(text[0] == 0); destroy_test();
}
/** Admission refusals and invalid commands preserve inspectable state; disposal never invents a new implicit root. */
static void it_reports_refusals_invalid_commands_and_disposed_host_access(void) {
    setup_test();
    require(tool_run_debugger("generated/ama-debugger", "generated/refused.jsonl",
        "resume\ninspect\nunknown\nrun\nresume\nresume\ninspect\nresume\ninspect\ndispose\ninspect\nrun\nquit\n") == EXIT_FAILURE);
    verify("refused", "generated/refused.jsonl"); destroy_test();
    setup_test();
    require(tool_run_debugger("generated/ama-debugger", "generated/empty.jsonl", "") == EXIT_SUCCESS);
    verify("empty", "generated/empty.jsonl"); destroy_test();
    setup_test();
    require(tool_run_debugger("generated/ama-debugger", "generated/final-line.jsonl", "run") == EXIT_SUCCESS);
    verify("final", "generated/final-line.jsonl"); destroy_test();
}
/** Existing response bytes cannot be truncated, even when commands would execute a project. */
static void it_refuses_existing_response_files_without_changing_them(void) {
    FILE *file = fopen("generated/existing.jsonl", "wb"); require(file != NULL);
    require(fputs("handwritten response\n", file) != EOF && fclose(file) == 0);
    setup_test(); require(tool_run_debugger("generated/ama-debugger", "generated/existing.jsonl", "run\n") == EXIT_FAILURE);
    verify("existing", "generated/existing.jsonl"); destroy_test();
    setup_test(); require(tool_run_debugger("generated/ama-debugger", "", "run\n") == EXIT_FAILURE); destroy_test();
    setup_test(); require(tool_run_debugger("generated/ama-debugger", "generated/absent/responses", "run\n") == EXIT_FAILURE); destroy_test();
}
/** An accepted native construction failure disposes state; inspection rejects rather than reconstructing dead objects. */
static void it_reports_real_native_failure_and_rejects_dead_state_inspection(void) {
    build("/test/fixture/v1.0.0/src/failed", "generated/failed-project", "generated/failed-debugger");
    setup_test();
    require(tool_run_debugger("generated/failed-debugger", "generated/failed.jsonl", "run\ninspect\nquit\n") == EXIT_FAILURE);
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
