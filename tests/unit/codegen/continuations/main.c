/**
 * @file
 * @brief Run generated continuation entry, yield, resume, copy and host-disposal scenarios.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../execution/common.c"
#include "support/acceptance.c"

/**
 * Assert that the component resumes repeated yields and restarts at entry. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_resumes_repeated_yields_and_restarts_at_entry(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |10\n%\nprobe !\n < |20\n%\nprobe !\n < |30\n");
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\ntask !\ntask %\ntask %\ntask %\n:\n task !\ntask !\n", "10:1:0\n20:2:0\n30:3:0\n10:4:0\n10:5:0\n", true);
}

/**
 * Assert that the component resumes inside a loop without preserving c activations. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_resumes_inside_a_loop_without_preserving_c_activations(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("decrement", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\ncount\n/test/fixture/v1.0.0/src/value\n |3\nstep\n/test/fixture/v1.0.0/src/decrement\ncount\n{\n step !\n  < count\n  > count\n %\n}\n");
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\ntask !\ntask %\ntask %\ntask %\n", "count=3\ncount=2\ncount=1\n", true);
}

/**
 * Assert that the component rejects unhandled root yield after cleanup. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_rejects_unhandled_root_yield_after_cleanup(void) {
    execute_project("@!\n%\n", "", false);
    expect_diagnostic("Unhandled root yield");
}

/**
 * Assert that the component exposes explicit host resume restart disposal and terminal failure.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_exposes_explicit_host_resume_restart_disposal_and_terminal_failure(void) {
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0}, other={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(result.outcome==AMA_HOST_YIELDED); AmaHandle root=host.root;\n"
        "host.executing=true; result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.status.code==AMA_BUSY);\n"
        "AmaStatus busy=ama_project_dispose(&host); assert(busy.code==AMA_BUSY); host.executing=false;\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "assert(ama_handle_equal(root,host.root));\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_COMPLETED);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_FAILED && result.status.code==AMA_NO_RESUME);\n"
        "assert(!host.disposed); AmaStatus disposed=ama_project_dispose(&host); assert(disposed.code==AMA_OK);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_FAILED);\n"
        "result=ama_project_execute(&other,AMA_ACCESS_ENTRY); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "AmaStatus status=ama_project_dispose(&other); assert(status.code==AMA_OK);\n"
        "status=ama_project_dispose(&other); assert(status.code==AMA_OK); return 0; }\n";
    execute_project("@<\ninput\n/test/fixture/v1.0.0/src/value\n |7\n@>\nresult\n/test/fixture/v1.0.0/src/value\n input\n@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < result\n > result\n%\nworker !\n < result\n > result\n%\n", "7:1:0\n8:2:0\n7:3:0\n10:4:0\n7:1:0\n", true);
    host_launcher = NULL;
}

/**
 * Assert that the component executes independent continuation copies and clears ordinary
 * positions. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_executes_independent_continuation_copies_and_clears_ordinary_positions(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |10\n%\nprobe !\n < |20\n%\nprobe !\n < |30\n");
    execute_project("@!\noriginal\n/test/fixture/v1.0.0/src/task\ncopy\n/test/fixture/v1.0.0/src/task\nplain\n/test/fixture/v1.0.0/src/task\nprobe\n/test/fixture/v1.0.0/src/worker\noriginal !\ncopy\n % original\nplain\n original\ncopy %\noriginal %\ncopy %\nplain %\n:\n probe !\n  < |90\n", "10:1:0\n20:2:0\n20:2:0\n30:3:0\n90:1:10\n", true);
}

/**
 * Assert that the component resumes both branch successors and transfers retained outputs. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_resumes_both_branch_successors_and_transfers_retained_outputs(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@<\ncondition\n/test/fixture/v1.0.0/src/value\n@>\nresult\n/test/fixture/v1.0.0/src/value\n |9\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\ncondition\n?\n %\n:\n %\nprobe !\n < result\n > result\n%\n");
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\nresult\n/test/fixture/v1.0.0/src/value\nprobe\n/test/fixture/v1.0.0/src/worker\ntask !\n < |1\n > result\ntask %\n > result\nprobe !\n < result\ntask %\ntask !\n < |0\ntask %\n > result\nprobe !\n < result\n", "9:1:0\n10:1:0\n9:2:0\n11:2:0\n", true);
}

/**
 * Assert that the component disposes yielded owned states and never resumes a failed execution.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_disposes_yielded_owned_states_and_never_resumes_a_failed_execution(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_YIELDED);\n"
        "AmaStatus status=ama_project_dispose(&host); assert(status.code==AMA_OK);\n"
        "assert(host.active.world==NULL && !host.registry.initialized);\n"
        "return 0; }\n";
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\ntask !\n%\ntask %\n", "1:1:0\n", true);
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_FAILED && host.disposed);\n"
        "assert(host.active.world==NULL && !host.registry.initialized);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_FAILED);\n"
        "return 0; }\n";
    execute_project("@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < |500\n%\n", "native-entered\n", true);
    host_launcher = NULL;
}

/**
 * Assert that the component reports the new caller after a yield releases the original link. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reports_the_new_caller_after_a_yield_releases_the_original_link(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nworker\n/test/fixture/v1.0.0/src/worker\n%\nworker !\n < |500\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/original", "@<\ntarget&\n/test/fixture/v1.0.0/src/task\n@!\ntarget& !\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/resumer", "@<\ntarget&\n/test/fixture/v1.0.0/src/task\n@!\ntarget& %\n");
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\noriginal\n/test/fixture/v1.0.0/src/original\nresumer\n/test/fixture/v1.0.0/src/resumer\noriginal !\n < &task\nresumer !\n < &task\n", "native-entered\n", false);
    expect_diagnostic("generated/project/packages/test/fixture/v1.0.0/src/task:5:1");
    expect_diagnostic("generated/project/packages/test/fixture/v1.0.0/src/resumer:5:1");
    FILE *errors = fopen("generated/errors.txt", "rb");
    assert(errors != NULL);
    char bytes[4096] = {0};
    size_t count = fread(bytes, 1, sizeof(bytes) - 1, errors);
    int closed = fclose(errors);
    assert(count != 0 && closed == 0);
    assert(strstr(bytes, "generated/project/packages/test/fixture/v1.0.0/src/original:") == NULL);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_runs_same_type_nested_copies_under_an_explicit_resuming_host();
    it_reports_only_the_current_caller_after_copying_a_continuation();
    it_resumes_repeated_yields_and_restarts_at_entry();
    it_resumes_inside_a_loop_without_preserving_c_activations();
    it_rejects_unhandled_root_yield_after_cleanup();
    it_exposes_explicit_host_resume_restart_disposal_and_terminal_failure();
    it_executes_independent_continuation_copies_and_clears_ordinary_positions();
    it_resumes_both_branch_successors_and_transfers_retained_outputs();
    it_disposes_yielded_owned_states_and_never_resumes_a_failed_execution();
    it_reports_the_new_caller_after_a_yield_releases_the_original_link();
}
/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) { run(); return EXIT_SUCCESS; }
