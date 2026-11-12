/**
 * @file
 * @brief Compile and execute generated Ama label programs and verify output/failure traces.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "common.c"

/** Ordinary self identity is handled after real selection; bad index conversion still fails. */
static void it_keeps_scalar_list_and_indexed_self_copies_checked_in_generated_execution(void) {
    execute_project("@!\nprobe\n.worker\nvalue\n.value\n |8\nitems[]\n.value\n |3\n |5\nindex\n.value\n |0\n"
        "value\n value\nitems[]\n items[]\nitems[index]\n items[index]\nprobe!\n < value\nprobe!\n < items[index]\n",
        "8:1:0\n3:2:0\n", true);
    execute_project("@!\nindex\n.value\n |9999\nitems[]\n.value\n |1\nitems[index]\n items[index]\n", "", false);
    expect_diagnostic("Index decoder failed");
}

/**
 * Assert that the component executes literal default retained and independent native states. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_executes_literal_default_retained_and_independent_native_states(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst!\n < |12\n > result\nfirst\n >> result\nfirst!\n <\n > result\nsecond!\n < |20\n > result\n", "12:1:0\n7:2:0\n20:1:0\n", true);
}

/**
 * Assert that the component does not enter the body or alternative after literal failure. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_does_not_enter_the_body_or_alternative_after_literal_failure(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nfirst!\n < |invalid\n:\n second!\n", "", false);
    expect_diagnostic("binding, type .value");
}

/**
 * Assert that the component routes outputs in text order using the new index. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_routes_outputs_in_text_order_using_the_new_index(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nindex\n.value\n |0\nitems[]\n.value\n |4\n |5\nfirst!\n input < |0\n output > index\n other > items[index]\nsecond!\n < items[index]\n", "0:1:0\n99:1:0\n", true);
}

/**
 * Assert that the component executes loops branches break and early return as label transfers.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_executes_loops_branches_break_and_early_return_as_label_transfers(void) {
    execute_project("@!\nfirst\n.worker\ncondition\n.value\n |1\nzero\n.value\n |0\ncondition\n{\n first!\n  < |3\n ^\n first!\n}\ncondition\n zero\ncondition\n?\n first!\n  < |4\n:\n first!\n  < |5\n>\nfirst!\n", "3:1:0\n5:2:0\n", true);
}

/**
 * Assert that the component nests distinct same type instances and keeps refusals in the caller
 * context. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_nests_distinct_same_type_instances_and_keeps_refusals_in_the_caller_context(void) {
    directory("generated/project");
    file("generated/project/node", "@<\nnext&\n.node\n@!\nprobe\n.worker\nnext&!\n:\n probe!\nprobe!\n");
    execute_project("@!\nfirst\n.node\nsecond\n.node\nfirst!\n < &second\nfirst!\n < &first\n", "7:1:7\n7:2:7\n7:1:0\n7:2:9\n7:3:9\n", true);
}

/**
 * Assert that the component copies lists during nested calls without relocating live native
 * state. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_copies_lists_during_nested_calls_without_relocating_live_native_state(void) {
    directory("generated/project");
    file("generated/project/node", "@!\nprobe\n.worker\nvalues[]\n.value\n |1\n |2\n |3\n |4\n |5\n |6\n |7\n |8\n |9\ncopies[]\n.value\ncopies[]\n values[]\nprobe!\n");
    execute_project("@!\nfirst\n.node\nsecond\n.node\nfirst!\nsecond!\nfirst!\n", "7:1:0\n7:1:0\n7:2:0\n", true);
}

/**
 * Assert that the component reads last refusal through an ordinary read only library module. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reads_last_refusal_through_an_ordinary_read_only_library_module(void) {
    directory("generated/project");
    native_fixture("inspect", "@<\nstatus\n.std.status\n@C\n");
    execute_project("@!\nabsent&\n.worker\nreader\n.std.refusal\ninspect\n.inspect\nstatus\n.std.status\nabsent&!\n:\n reader!\n  > status\n inspect!\n  < status\nreader!\n > status\ninspect!\n < status\n", "status=7\nstatus=7\n", true);
}

/**
 * Assert that the component diagnoses unicode execution paths and failures before root entry.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_diagnoses_unicode_execution_paths_and_failures_before_root_entry(void) {
    directory("generated/project");
    file("generated/project/节点", "@!\nworker\n.worker\nworker!\n < |bad\n");
    execute_project("@!\n树\n.节点\n树!\n", "", false);
    expect_diagnostic("generated/project/节点:5:2");
    expect_diagnostic("generated/project/main:4:1");
    execute_project("@=\nbad\n.value\n |bad\n@!\nworker\n.worker\nworker!\n", "", false);
    expect_diagnostic("shared construction, type .value");
}

/**
 * Assert that the component handles target bounds refusal but not index decoder failure. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_handles_target_bounds_refusal_but_not_index_decoder_failure(void) {
    execute_project("@!\nindex\n.value\n |20\nitems[]\n.worker\nseed\n.worker\nitems[index]!\n:\n seed!\n", "7:1:12\n", true);
    execute_project("@!\nindex\n.value\n |9999\nitems[]\n.worker\nseed\n.worker\nitems[index]!\n:\n seed!\n", "", false);
    expect_diagnostic("Index decoder failed");
}

/**
 * Assert that the component rejects a surviving native entry before publishing any executable.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_a_surviving_native_entry_before_publishing_any_executable(void) {
    noninline_entry = true;
    prepare_project("@!\nfirst\n.worker\nfirst!\n");
    noninline_entry = false;
    FILE *errors = tmpfile();
    assert(errors != NULL);
    bool compiled = project_codegen_compile(&assembly, errors);
    int closed = fclose(errors);
    assert(!compiled && closed == 0);
    assert(strstr(assembly.error.message,"not inlined")!=NULL);
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/**
 * Assert that the component selects named inputs in text order but installs in declaration
 * order. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_selects_named_inputs_in_text_order_but_installs_in_declaration_order(void) {
    directory("generated/project");
    native_fixture("ordered","@<\na\n.value\n |1\nb\n.value\n a\n@>\nresult\n.value\n b\n@C\n");
    execute_project("@!\nordered\n.ordered\nfirst\n.value\n |1000\nsecond\n.value\n |1001\nitems[]\n.value\n |3\n |8\nordered!\n b < items[second]\n a < items[first]\n", "index=1001\nindex=1000\nordered=3,8,8\n",true);
}

/**
 * Assert that the component reads initial and last results without reset or body execution. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reads_initial_and_last_results_without_reset_or_body_execution(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst\n >> result\nsecond!\n < result\nfirst!\n < |12\nfirst\n >> result\nsecond!\n < result\nfirst!\n", "7:1:0\n12:1:0\n13:2:0\n7:2:0\n",true);
}

/**
 * Assert that the component writes repeated receivers in binding text order. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_writes_repeated_receivers_in_binding_text_order(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst!\n input < |4\n other > result\n output > result\nsecond!\n < result\n", "4:1:0\n5:1:0\n",true);
}

/**
 * Assert that the component refuses resume without preparing the interface. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_refuses_resume_without_preparing_the_interface(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nfirst%\n:\n second!\n", "7:1:10\n",true);
}

/**
 * Assert that the component reports failure with no available runtime heap or root frame. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reports_failure_with_no_available_runtime_heap_or_root_frame(void) {
    file("generated/fault_allocator.h", "#include <stdlib.h>\n#define malloc(size) ((void)(size), (void *)0)\n");
    forced_header="generated/fault_allocator.h";
    execute_project("@!\nfirst\n.worker\nfirst!\n", "", false);
    forced_header=NULL;
    expect_diagnostic("Ama runtime failure: 2");
    expect_diagnostic("generated/project/main:1:1");
}

/**
 * Assert that the component repeats a loop and returns from a callee to the right caller. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_repeats_a_loop_and_returns_from_a_callee_to_the_right_caller(void) {
    directory("generated/project");
    native_fixture("decrement","@<\ninput\n.value\n@>\noutput\n.value\n@C\n");
    file("generated/project/returner","@!\nflag\n.value\n |1\nworker\n.worker\nflag\n?\n >\nworker!\n");
    execute_project("@!\ncounter\n.value\n |3\nstep\n.decrement\nearly\n.returner\nearly!\ncounter\n{\n step!\n  < counter\n  > counter\n}\n", "count=3\ncount=2\ncount=1\n",true);
}

/**
 * Assert that the component reads shared namespaces and qualified values without default
 * instances. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_reads_shared_namespaces_and_qualified_values_without_default_instances(void) {
    directory("generated/project");
    file("generated/project/numbers","@=\nfirst\n.value\n |5\nsecond\n.value\n |8\n");
    execute_project("@=\nshared\n.value\n |42\n@!\nworker\n.worker\nresult\n.value\nworker!\n < .numbers.first\n.numbers\n >> result\n >> result\nworker!\n < result\nworker!\n < .main.shared\n", "5:1:0\n8:2:0\n42:3:0\n",true);
}

/**
 * Assert that the component checks the exact native entry signature including its return type.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_checks_the_exact_native_entry_signature_including_its_return_type(void) {
    wrong_entry_result = true;
    prepare_project("@!\nfirst\n.worker\nfirst!\n");
    wrong_entry_result = false;
    FILE *errors = tmpfile();
    assert(errors != NULL);
    bool compiled = project_codegen_compile(&assembly, errors);
    int closed = fclose(errors);
    assert(!compiled && closed == 0 && !assembly.ready && !assembly.published);
    assert(strstr(assembly.error.message,"native toolchain failed")!=NULL);
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/**
 * Assert that the component allows ordinary native helper calls inside an inlined entry. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_allows_ordinary_native_helper_calls_inside_an_inlined_entry(void) {
    directory("generated/project");
    native_fixture("recursive","@<\ninput\n.value\n@>\noutput\n.value\n@C\n");
    execute_project("@!\nworker\n.recursive\nworker!\n < |4\n", "sum=10\n",true);
}

/**
 * Assert that the component reports native body errors with the active ama chain before cleanup.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_reports_native_body_errors_with_the_active_ama_chain_before_cleanup(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nfirst!\n < |500\n:\n second!\n", "native-entered\n", false);
    expect_diagnostic("Body failed");
    expect_diagnostic("native entry, type .worker");
    expect_diagnostic("generated/project/main:6:1");
}

/**
 * Assert that the component grows the current module type pool while its caller waits. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_grows_the_current_module_type_pool_while_its_caller_waits(void) {
    directory("generated/project");
    file("generated/project/node","@<\ninput[]\n.node\n@!\ncopies[]\n.node\nprobe\n.worker\ncopies[]\n input[]\nprobe!\n");
    execute_project("@!\nseed\n.node\nworker\n.node\nitems[]\n.node\n seed\n seed\n seed\n seed\n seed\n seed\n seed\n seed\n seed\nworker!\n < items[]\nworker!\n < items[]\n", "7:1:0\n7:2:0\n",true);
}

/**
 * Assert that the component reports the complete active chain before terminal disposal. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_reports_the_complete_active_chain_before_terminal_disposal(void) {
    directory("generated/project");
    file("generated/project/outer","@!\nchild\n.inner\nchild!\n");
    file("generated/project/inner","@!\nworker\n.worker\nworker!\n < |500\n");
    execute_project("@!\nouter\n.outer\nouter!\n", "native-entered\n",false);
    expect_diagnostic("generated/project/worker:1:1");
    expect_diagnostic("generated/project/inner:4:1");
    expect_diagnostic("generated/project/outer:4:1");
    expect_diagnostic("generated/project/main:4:1");
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_keeps_scalar_list_and_indexed_self_copies_checked_in_generated_execution();
    it_executes_literal_default_retained_and_independent_native_states();
    it_does_not_enter_the_body_or_alternative_after_literal_failure();
    it_routes_outputs_in_text_order_using_the_new_index();
    it_executes_loops_branches_break_and_early_return_as_label_transfers();
    it_nests_distinct_same_type_instances_and_keeps_refusals_in_the_caller_context();
    it_copies_lists_during_nested_calls_without_relocating_live_native_state();
    it_reads_last_refusal_through_an_ordinary_read_only_library_module();
    it_diagnoses_unicode_execution_paths_and_failures_before_root_entry();
    it_handles_target_bounds_refusal_but_not_index_decoder_failure();
    it_rejects_a_surviving_native_entry_before_publishing_any_executable();
    it_selects_named_inputs_in_text_order_but_installs_in_declaration_order();
    it_reads_initial_and_last_results_without_reset_or_body_execution();
    it_writes_repeated_receivers_in_binding_text_order();
    it_refuses_resume_without_preparing_the_interface();
    it_reports_failure_with_no_available_runtime_heap_or_root_frame();
    it_repeats_a_loop_and_returns_from_a_callee_to_the_right_caller();
    it_reads_shared_namespaces_and_qualified_values_without_default_instances();
    it_checks_the_exact_native_entry_signature_including_its_return_type();
    it_allows_ordinary_native_helper_calls_inside_an_inlined_entry();
    it_reports_native_body_errors_with_the_active_ama_chain_before_cleanup();
    it_grows_the_current_module_type_pool_while_its_caller_waits();
    it_reports_the_complete_active_chain_before_terminal_disposal();
}
/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) { run(); return EXIT_SUCCESS; }
