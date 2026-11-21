/** @file
 * @brief Check complete selection validation, isolated executions and byte-exact evidence.
 * @see [T233 contract](../../../../../docs/implementation/project-tool.md)
 */
#define _XOPEN_SOURCE 700
#include <signal.h>
#include "../common.c"
#include "../../../../../tools/project/testing/testing.h"
#include "../../../../../tools/project/process/process.h"
static size_t executions;
static bool infrastructure;
static char *self;
/** A real independently executed process supplies the observation, not a mock status. */
static bool execute(void *context, const ProjectTestJob *job, const char *directory, ProjectTestOutcome *outcome) {
    (void)context;
    executions++;
    if (infrastructure) { return false; }
    assert(members(directory) == 1);
    FILE *output = tmpfile(), *errors = tmpfile(); assert(output && errors);
    char *arguments[] = {self, "child", (char *)job->arguments[0], (char *)job->arguments[1], NULL};
    ProjectProcessResult result;
    bool observed = project_process(arguments, directory, output, errors, &result);
    if (observed && !result.error && !result.signal) {
        int moved = fseek(output, 0, SEEK_SET); assert(!moved);
        for (const char *byte = job->arguments[1]; *byte; byte++) {
            int received = fgetc(output); assert(received == (unsigned char)*byte);
        }
        int end = fgetc(output); assert(end == EOF);
    }
    int closed = fclose(output); assert(!closed); closed = fclose(errors); assert(!closed);
    if (!observed || result.error) { return false; }
    *outcome = (ProjectTestOutcome){.stage = PROJECT_TEST_COMPLETE, .exit_code = result.exit_code, .signal = result.signal};
    return true;
}
/** Describe one prepared test selection while borrowing its exact argument bytes. */
static ProjectTestJob job(const char *name, const char *const *args) {
    return (ProjectTestJob){name, "/test/fixture/v1.0.0/tests/probe", args, 2,
        "prepared source hash: fixture\ncompiler edition: current\n"};
}
/** Parse the independently specified decimal byte record, preserving embedded newlines. */
static void check_field(FILE *file, const char *expected) {
    size_t length; int fields = fscanf(file, "%zu:", &length); assert(fields == 1 && length == strlen(expected));
    for (size_t index = 0; index < length; index++) { int byte = fgetc(file); assert(byte == (unsigned char)expected[index]); }
    int byte = fgetc(file); assert(byte == ','); byte = fgetc(file); assert(byte == '\n');
}
/** Require separate process state and lossless records across success, failure and signal. */
static void it_runs_distinct_microprograms_and_records_exact_arguments_and_failures(void) {
    setup(); executions = 0; infrastructure = false;
    const char *first[] = {"0", "empty spaces; $(literal)\nnext"}, *second[] = {"7", ""}, *third[] = {"signal", "literal"};
    ProjectTestJob jobs[] = {job("first", first), job("second", second), job("signal", third)};
    ProjectTestRequest request = {root, root, jobs, 3, NULL, execute, NULL, 0};
    ProjectTestOutcome outcomes[3] = {{0}};
    ProjectTestResult result = project_tests(&request, allocator(), outcomes);
    assert(!result.success && result.completed == 3 && executions == 3 && result.directory);
    assert(outcomes[0].exit_code == 0 && outcomes[1].exit_code == 7);
    assert(outcomes[2].signal == SIGTERM && !outcomes[2].exit_code);
    assert(outcomes[0].stage == PROJECT_TEST_COMPLETE && outcomes[1].stage == PROJECT_TEST_COMPLETE);
    char path[512]; (void)snprintf(path, sizeof(path), "%s/0/request.record", result.directory);
    FILE *record = fopen(path, "rb"); assert(record);
    check_field(record, "ama.test.request.v1"); check_field(record, "first");
    check_field(record, jobs[0].module); check_field(record, jobs[0].provenance);
    int count = 0; int scanned = fscanf(record, "%d", &count); assert(scanned == 1 && count == 2);
    int byte = fgetc(record); assert(byte == '\n');
    check_field(record, first[0]); check_field(record, first[1]);
    byte = fgetc(record); assert(byte == EOF); int closed = fclose(record); assert(!closed);
    (void)snprintf(path, sizeof(path), "%s/1/outcome.txt", result.directory);
    assert_file(path, "stage=3\nexit=7\nsignal=0\n");
    release(NULL, result.directory); destroy();
}
/** Reject the entire invalid plan before adapters or filesystem publication. */
static void it_rejects_conflicting_or_invalid_selections_before_any_output(void) {
    setup(); executions = 0;
    const char *args[] = {"0", ""};
    ProjectTestJob jobs[] = {job("same", args), job("same", args)};
    ProjectTestRequest request = {root, root, jobs, 2, NULL, execute, NULL, 0};
    ProjectTestOutcome outcomes[2] = {{.exit_code = 42}, {.exit_code = 42}};
    for (int invalid = 0; invalid < 5; invalid++) {
        if (invalid == 1) { jobs[1].name = "other"; jobs[1].module = "../not canonical"; }
        if (invalid == 2) { jobs[1].module = jobs[0].module; jobs[1].arguments = NULL; }
        if (invalid == 3) { jobs[1].arguments = args; jobs[1].provenance = NULL; }
        if (invalid == 4) { jobs[1].provenance = "known"; request.execute = NULL; }
        ProjectTestResult result = project_tests(&request, allocator(), outcomes);
        assert(!result.success && !result.directory && !result.completed && !executions);
        assert(outcomes[0].exit_code == 42 && outcomes[1].exit_code == 42 && members(root) == 0);
    }
    destroy();
}
/** Exclude every physical source namespace while allowing true sibling paths. */
static void it_keeps_output_outside_both_source_namespaces_and_physical_aliases(void) {
    setup(); executions = 0;
    char packages[256], dependencies[256], alias[256];
    (void)snprintf(packages, sizeof(packages), "%s/packages", root);
    (void)snprintf(dependencies, sizeof(dependencies), "%s/dependencies", root);
    (void)snprintf(alias, sizeof(alias), "%s/alias", root);
    int made = mkdir(packages, 0700); assert(!made); made = mkdir(dependencies, 0700); assert(!made);
    int linked = symlink("packages", alias); assert(!linked);
    const char *args[] = {"0", ""}; ProjectTestJob selected = job("one", args);
    ProjectTestRequest request = {root, packages, &selected, 1, NULL, execute, NULL, 0}; ProjectTestOutcome outcome = {0};
    const char *paths[] = {packages, dependencies, alias};
    for (size_t index = 0; index < 3; index++) {
        request.output_parent = paths[index];
        ProjectTestResult result = project_tests(&request, allocator(), &outcome);
        assert(!result.success && !result.directory && !executions);
        assert(members(packages) == 0 && members(dependencies) == 0);
    }
    /* A Standard/extra root is a source namespace even outside packages. */
    const char *additional[] = {root};
    request.source_roots = additional; request.source_root_count = 1; request.output_parent = root;
    ProjectTestResult excluded = project_tests(&request, allocator(), &outcome);
    assert(!excluded.success && !excluded.directory && !executions);
    request.source_root_count = 0; request.source_roots = NULL;
    char sibling[256]; (void)snprintf(sibling, sizeof(sibling), "%s/packages sibling", root);
    made = mkdir(sibling, 0700); assert(!made);
    request.output_parent = sibling;
    ProjectTestResult result = project_tests(&request, allocator(), &outcome);
    assert(result.success && executions == 1 && result.directory);
    release(NULL, result.directory);
    destroy();
}
/** Preserve partial reports and balanced ownership on infrastructure refusal. */
static void it_retains_partial_evidence_when_infrastructure_or_allocation_fails(void) {
    setup(); executions = 0; infrastructure = true;
    const char *args[] = {"0", ""}; ProjectTestJob selected = job("one", args);
    ProjectTestRequest request = {root, root, &selected, 1, NULL, execute, NULL, 0}; ProjectTestOutcome outcome = {.exit_code = 42};
    ProjectTestResult result = project_tests(&request, allocator(), &outcome);
    assert(!result.success && result.directory && result.completed == 0 && executions == 1 && outcome.exit_code == 42);
    release(NULL, result.directory); infrastructure = false; destroy();
    setup(); executions = 0;
    result = project_tests(&request, allocator(), &outcome);
    assert(result.success); size_t count = allocations; release(NULL, result.directory); destroy();
    for (size_t failure = 1; failure <= count; failure++) {
        setup(); fail_allocation = failure;
        result = project_tests(&request, allocator(), &outcome);
        assert(!result.success);
        release(NULL, result.directory); destroy();
    }
}
/** Distinguish launch errors from real exit statuses and signal termination. */
static void it_distinguishes_exec_failure_normal_127_and_signal_termination(void) {
    char *missing[] = {"/nonexistent/ama-tool", NULL};
    ProjectProcessResult result = {.exit_code = 42};
    bool observed = project_process(missing, NULL, NULL, NULL, &result);
    assert(observed && result.error == ENOENT && result.exit_code == 127 && !result.signal);
    char *normal[] = {self, "child", "127", "literal", NULL};
    observed = project_process(normal, NULL, NULL, NULL, &result);
    assert(observed && !result.error && result.exit_code == 127 && !result.signal);
    normal[2] = "signal";
    observed = project_process(normal, NULL, NULL, NULL, &result);
    assert(observed && !result.error && result.signal == SIGTERM);
    normal[2] = "0";
    observed = project_process(normal, "/nonexistent/ama-directory", NULL, NULL, &result);
    assert(observed && result.error == ENOENT);
    observed = project_process(NULL, NULL, NULL, NULL, &result); assert(!observed);
}
/** Execute every named assertion case explicitly in the intended order. */
void run(void) {
    it_runs_distinct_microprograms_and_records_exact_arguments_and_failures();
    it_rejects_conflicting_or_invalid_selections_before_any_output();
    it_keeps_output_outside_both_source_namespaces_and_physical_aliases();
    it_retains_partial_evidence_when_infrastructure_or_allocation_fails();
    it_distinguishes_exec_failure_normal_127_and_signal_termination();
}
/** Adapt the harness process to its explicit assertion cases and child controls. */
int main(int count, char **arguments) {
    if (count == 4 && !strcmp(arguments[1], "child")) {
        if (!strcmp(arguments[2], "signal")) { (void)raise(SIGTERM); return 99; }
        /* A fresh process must not inherit the parent harness's mutable state. */
        if (executions) { return 98; }
        FILE *state = !strcmp(arguments[3], "literal") ? tmpfile() : fopen("state", "wx");
        if (!state) { return 97; }
        int closed = fclose(state); if (closed) { return 96; }
        int printed = printf("%s", arguments[3]); if (printed < 0) { return 95; }
        return atoi(arguments[2]);
    }
    self = realpath(arguments[0], NULL); assert(self);
    run(); free(self); return 0;
}
