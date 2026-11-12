/**
 * @file
 * @brief Run generated continuation entry, yield, resume, copy and host-disposal scenarios.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../execution/common.c"

/**
 * Assert that the component resumes repeated yields and restarts at entry. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_resumes_repeated_yields_and_restarts_at_entry(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe!\n < |10\n%\nprobe!\n < |20\n%\nprobe!\n < |30\n");
    execute_project("@!\ntask\n.task\ntask!\ntask%\ntask%\ntask%\n:\n task!\ntask!\n", "10:1:0\n20:2:0\n30:3:0\n10:4:0\n10:5:0\n", true);
}

/**
 * Assert that the component resumes inside a loop without preserving c activations. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_resumes_inside_a_loop_without_preserving_c_activations(void) {
    directory("generated/project");
    native_fixture("decrement", "@<\ninput\n.value\n@>\noutput\n.value\n@C\n");
    file("generated/project/task", "@!\ncount\n.value\n |3\nstep\n.decrement\ncount\n{\n step!\n  < count\n  > count\n %\n}\n");
    execute_project("@!\ntask\n.task\ntask!\ntask%\ntask%\ntask%\n", "count=3\ncount=2\ncount=1\n", true);
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
        "result=ama_project_execute(&other,AMA_ACCESS_ENTRY); assert(result.status.code==AMA_BUSY);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "assert(ama_handle_equal(root,host.root));\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_COMPLETED);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_FAILED && result.status.code==AMA_NO_RESUME);\n"
        "assert(host.disposed); result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_FAILED);\n"
        "result=ama_project_execute(&other,AMA_ACCESS_ENTRY); assert(result.outcome==AMA_HOST_YIELDED);\n"
        "AmaStatus status=ama_project_dispose(&other); assert(status.code==AMA_OK);\n"
        "status=ama_project_dispose(&other); assert(status.code==AMA_OK); return 0; }\n";
    execute_project("@<\ninput\n.value\n |7\n@>\nresult\n.value\n input\n@!\nworker\n.worker\nworker!\n < result\n > result\n%\nworker!\n < result\n > result\n%\n", "7:1:0\n8:2:0\n7:3:0\n10:4:0\n7:1:0\n", true);
    host_launcher = NULL;
}

/**
 * Assert that the component executes independent continuation copies and clears ordinary
 * positions. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_executes_independent_continuation_copies_and_clears_ordinary_positions(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe!\n < |10\n%\nprobe!\n < |20\n%\nprobe!\n < |30\n");
    execute_project("@!\noriginal\n.task\ncopy\n.task\nplain\n.task\nprobe\n.worker\noriginal!\ncopy\n %original\nplain\n original\ncopy%\noriginal%\ncopy%\nplain%\n:\n probe!\n  < |90\n", "10:1:0\n20:2:0\n20:2:0\n30:3:0\n90:1:10\n", true);
}

/**
 * Assert that the component resumes both branch successors and transfers retained outputs. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_resumes_both_branch_successors_and_transfers_retained_outputs(void) {
    directory("generated/project");
    file("generated/project/task", "@<\ncondition\n.value\n@>\nresult\n.value\n |9\n@!\nprobe\n.worker\ncondition\n?\n %\n:\n %\nprobe!\n < result\n > result\n%\n");
    execute_project("@!\ntask\n.task\nresult\n.value\nprobe\n.worker\ntask!\n < |1\n > result\ntask%\n > result\nprobe!\n < result\ntask%\ntask!\n < |0\ntask%\n > result\nprobe!\n < result\n", "9:1:0\n10:1:0\n9:2:0\n11:2:0\n", true);
}

/**
 * Assert that the component disposes yielded owned states and never resumes a failed execution.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_disposes_yielded_owned_states_and_never_resumes_a_failed_execution(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe!\n < |1\n%\nprobe!\n < |2\n");
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_YIELDED);\n"
        "AmaStatus status=ama_project_dispose(&host); assert(status.code==AMA_OK);\n"
        "for(size_t i=0;i<host.world.type_count;i++) assert(host.world.types[i]->pool.live_count==0);\n"
        "return 0; }\n";
    execute_project("@!\ntask\n.task\ntask!\n%\ntask%\n", "1:1:0\n", true);
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_FAILED && host.disposed);\n"
        "for(size_t i=0;i<host.world.type_count;i++) assert(host.world.types[i]->pool.live_count==0);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.outcome==AMA_HOST_FAILED);\n"
        "return 0; }\n";
    execute_project("@!\nworker\n.worker\nworker!\n < |500\n%\n", "native-entered\n", true);
    host_launcher = NULL;
}

/**
 * Assert that the component reports the new caller after a yield releases the original link. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reports_the_new_caller_after_a_yield_releases_the_original_link(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nworker\n.worker\n%\nworker!\n < |500\n");
    file("generated/project/original", "@<\ntarget&\n.task\n@!\ntarget&!\n");
    file("generated/project/resumer", "@<\ntarget&\n.task\n@!\ntarget&%\n");
    execute_project("@!\ntask\n.task\noriginal\n.original\nresumer\n.resumer\noriginal!\n < &task\nresumer!\n < &task\n", "native-entered\n", false);
    expect_diagnostic("generated/project/task:5:1");
    expect_diagnostic("generated/project/resumer:5:1");
    FILE *errors = fopen("generated/errors.txt", "rb");
    assert(errors != NULL);
    char bytes[4096] = {0};
    size_t count = fread(bytes, 1, sizeof(bytes) - 1, errors);
    int closed = fclose(errors);
    assert(count != 0 && closed == 0);
    assert(strstr(bytes, "generated/project/original:") == NULL);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
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
