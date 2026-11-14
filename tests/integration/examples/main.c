/** @file
 * @brief Execute readable published Ama examples and compare their complete effects.
 */
#include "common.c"
/** Copied continuations visit separate owned lists from the same saved boundary. */
static void it_runs_independent_simulations(void) {
    compile_and_run_object(".simulations", "description=first\ndescription=first\ndescription=second\ndescription=second\n", NULL, false);
}
/** Future appends are visited; removing the current entry preserves successors. */
static void it_mutates_a_live_traversal_and_skips_the_empty_list(void) {
    compile_and_run_object(".traversal", "description=first\ndescription=second\ndescription=appended\n", NULL, false);
}
/** A returned self-reference can enter its original instance after release. */
static void it_executes_a_returned_self_reference(void) {
    compile_and_run_object(".self example", "description=self entry\ndescription=self entry\n", NULL, false);
}
/** Failure inspection is retained data, and a subsequent successful call clears it. */
static void it_handles_file_failure_and_later_success(void) {
    compile_and_run_object(".files", "description=Cannot open file\ndescription=Cannot open file\ndescription=opened\n", NULL, false);
}
/** Exercise the same native/runtime lifetimes under target instrumentation. */
static void it_runs_examples_with_target_sanitizers(void) {
    compile_and_run_object(".simulations", "description=first\ndescription=first\ndescription=second\ndescription=second\n", NULL, true);
    compile_and_run_object(".traversal", "description=first\ndescription=second\ndescription=appended\n", NULL, true);
    compile_and_run_object(".files", "description=Cannot open file\ndescription=Cannot open file\ndescription=opened\n", NULL, true);
}
/** Empty traversal finishes, empty path is data, and a data-only clone cannot resume. */
static void it_checks_empty_inputs_and_cleared_copy_positions(void) {
    compile_and_run_object(".empty", "", NULL, false);
    compile_and_run_object(".empty path", "description=Invalid file path\n", NULL, false);
    compile_and_run_object(".ordinary copy", "", "Ama runtime failure: 10", false);
}
/** The real CLI exposes exact operation, ownership witness and native ABI locations. */
static void it_explains_the_transitive_copy_failure_with_source_navigation(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--check", "project", ".copy failure",
        "--standard-root", TEST_STANDARD_ROOT, NULL};
    int result = run_compiler_arguments(arguments);
    char diagnostic[8192], actual[128];
    read_stream(errors, diagnostic, sizeof(diagnostic));
    read_stream(output, actual, sizeof(actual));
    assert(result != EXIT_SUCCESS && actual[0] == '\0');
    assert(strstr(diagnostic, "project/copy failure:6:2: error[type-mismatch]") != NULL);
    assert(strstr(diagnostic, "project/uncopyable simulation:3:1: note: owned field requiring independent copying") != NULL);
    assert(strstr(diagnostic, "project/open.abi:3:1: note: type capability contract") != NULL);
    assert(strstr(diagnostic, "cause: An owned native type has no declared = copy capability") != NULL);
    assert(strstr(diagnostic, "help: Independent copying requires =") != NULL);
    destroy_test();
}
/** Register every complete example explicitly. */
void run(void) {
    it_explains_the_transitive_copy_failure_with_source_navigation();
    it_runs_independent_simulations();
    it_mutates_a_live_traversal_and_skips_the_empty_list();
    it_executes_a_returned_self_reference();
    it_handles_file_failure_and_later_success();
    it_runs_examples_with_target_sanitizers();
    it_checks_empty_inputs_and_cleared_copy_positions();
}
/** Return after all compilation, output, status and cleanup assertions pass. */
int main(void) { run(); return EXIT_SUCCESS; }
