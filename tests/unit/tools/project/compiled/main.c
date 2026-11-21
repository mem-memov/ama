/** @file
 * @brief Demonstrate two independently compiled/executed Ama test selections.
 * @see [T233 contract](../../../../../docs/implementation/project-tool.md)
 */
#include "../common.c"
#include "../../../../../tools/project/testing/testing.h"
#include "../../../../../tools/project/process/process.h"
/** This acceptance adapter uses the existing explicit low-level compilation
 * boundary. T242 replaces the composition with the accepted CMake adapter; no
 * production high-level build command is claimed by this fixture. */
static bool execute(void *context, const ProjectTestJob *job, const char *directory, ProjectTestOutcome *outcome) {
    (void)context;
    assert(members(directory) == 1);
    char compiled[512], out[512], err[512], program[512];
    (void)snprintf(compiled, sizeof(compiled), "%s/compiled", directory);
    (void)snprintf(out, sizeof(out), "%s/compile.stdout.log", directory);
    (void)snprintf(err, sizeof(err), "%s/compile.stderr.log", directory);
    FILE *output = fopen(out, "wb"), *errors = fopen(err, "wb"); assert(output && errors);
    char *arguments[] = {AMA_EXECUTABLE, "--emit", TEST_PROJECT, (char *)job->module,
        "--out", compiled, "--sdk", TEST_SDK, "--compile", NULL};
    ProjectProcessResult result = {.exit_code = 1};
    bool observed = project_process(arguments, NULL, output, errors, &result);
    *outcome = (ProjectTestOutcome){.stage = PROJECT_TEST_COMPILATION,
        .exit_code = result.exit_code, .signal = result.signal};
    if (!observed || result.error) { (void)fclose(output); (void)fclose(errors); return false; }
    if (!result.exit_code && !result.signal) {
        int closed = fclose(output); assert(!closed); closed = fclose(errors); assert(!closed);
        (void)snprintf(out, sizeof(out), "%s/stdout.log", directory);
        (void)snprintf(err, sizeof(err), "%s/stderr.log", directory);
        output = fopen(out, "wb"); errors = fopen(err, "wb"); assert(output && errors);
        int length = snprintf(program, sizeof(program), "%s/program", compiled); assert(length > 0 && (size_t)length < sizeof(program));
        char *run[] = {program, NULL};
        observed = project_process(run, directory, output, errors, &result);
        *outcome = (ProjectTestOutcome){.stage = PROJECT_TEST_COMPLETE,
            .exit_code = result.exit_code, .signal = result.signal};
    }
    int closed = fclose(output); assert(!closed); closed = fclose(errors); assert(!closed);
    return observed && !result.error;
}
/** Compile and run two selected Ama tests in independently owned output trees. */
static void it_compiles_and_runs_each_ama_microprogram_in_its_own_output_tree(void) {
    setup();
    ProjectTestJob jobs[] = {
        {.name = "first", .module = "/test/fixture/v1.0.0/tests/first", .provenance = "tests/first sha256=5b51ddf6c0a090710eb04a02a369c75f27cad228e3d1acb3c8a48a5c44a99c3b; package.ini sha256=e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855; src/helper sha256=96d4fe567987b13c6686bfc52b36de77f858a1150b2614287c6d113befe22a67; native=none; aliases=none; compiler and SDK from explicit acceptance build"},
        {.name = "second", .module = "/test/fixture/v1.0.0/tests/second", .provenance = "tests/second sha256=5b51ddf6c0a090710eb04a02a369c75f27cad228e3d1acb3c8a48a5c44a99c3b; package.ini sha256=e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855; src/helper sha256=96d4fe567987b13c6686bfc52b36de77f858a1150b2614287c6d113befe22a67; native=none; aliases=none; compiler and SDK from explicit acceptance build"}
    };
    ProjectTestRequest request = {TEST_PROJECT, root, jobs, 2, NULL, execute, NULL, 0};
    ProjectTestOutcome outcomes[2];
    ProjectTestResult result = project_tests(&request, allocator(), outcomes);
    assert(result.success && result.completed == 2 && result.directory);
    for (size_t index = 0; index < 2; index++) {
        assert(outcomes[index].stage == PROJECT_TEST_COMPLETE && !outcomes[index].exit_code && !outcomes[index].signal);
        char path[512]; (void)snprintf(path, sizeof(path), "%s/%zu/stdout.log", result.directory, index); assert_file(path, "");
        (void)snprintf(path, sizeof(path), "%s/%zu/stderr.log", result.directory, index); assert_file(path, "");
    }
    release(NULL, result.directory); destroy();
}
/** Execute every named assertion case explicitly in the intended order. */
void run(void) { it_compiles_and_runs_each_ama_microprogram_in_its_own_output_tree(); }
/** Adapt the harness process to its explicit assertion cases and child controls. */
int main(void) { run(); return 0; }
