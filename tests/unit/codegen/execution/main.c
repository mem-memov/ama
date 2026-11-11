#include "common.c"

static void it_executes_literal_default_retained_and_independent_native_states(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst!\n < |12\n > result\nfirst\n >> result\nfirst!\n <\n > result\nsecond!\n < |20\n > result\n", "12:1:0\n7:2:0\n20:1:0\n", true);
}

static void it_does_not_enter_the_body_or_alternative_after_literal_failure(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nfirst!\n < |invalid\n:\n second!\n", "", false);
    expect_diagnostic("binding, type .value");
}

static void it_routes_outputs_in_text_order_using_the_new_index(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nindex\n.value\n |0\nitems[]\n.value\n |4\n |5\nfirst!\n input < |0\n output > index\n other > items[index]\nsecond!\n < items[index]\n", "0:1:0\n99:1:0\n", true);
}

static void it_executes_loops_branches_break_and_early_return_as_label_transfers(void) {
    execute_project("@!\nfirst\n.worker\ncondition\n.value\n |1\nzero\n.value\n |0\ncondition\n{\n first!\n  < |3\n ^\n first!\n}\ncondition\n zero\ncondition\n?\n first!\n  < |4\n:\n first!\n  < |5\n>\nfirst!\n", "3:1:0\n5:2:0\n", true);
}

static void it_nests_distinct_same_type_instances_and_keeps_refusals_in_the_caller_context(void) {
    directory("generated/project");
    file("generated/project/node", "@<\nnext&\n.node\n@!\nprobe\n.worker\nnext&!\n:\n probe!\nprobe!\n");
    execute_project("@!\nfirst\n.node\nsecond\n.node\nfirst!\n < &second\nfirst!\n < &first\n", "7:1:7\n7:2:7\n7:1:0\n7:2:9\n7:3:9\n", true);
}

static void it_copies_lists_during_nested_calls_without_relocating_live_native_state(void) {
    directory("generated/project");
    file("generated/project/node", "@!\nprobe\n.worker\nvalues[]\n.value\n |1\n |2\n |3\n |4\n |5\n |6\n |7\n |8\n |9\ncopies[]\n.value\ncopies[]\n values[]\nprobe!\n");
    execute_project("@!\nfirst\n.node\nsecond\n.node\nfirst!\nsecond!\nfirst!\n", "7:1:0\n7:1:0\n7:2:0\n", true);
}

static void it_reads_last_refusal_through_an_ordinary_read_only_library_module(void) {
    directory("generated/project");
    native_fixture("inspect", "@<\nstatus\n.std.status\n@C\n");
    execute_project("@!\nabsent&\n.worker\nreader\n.std.refusal\ninspect\n.inspect\nstatus\n.std.status\nabsent&!\n:\n reader!\n  > status\n inspect!\n  < status\nreader!\n > status\ninspect!\n < status\n", "status=7\nstatus=7\n", true);
}

static void it_diagnoses_unicode_execution_paths_and_failures_before_root_entry(void) {
    directory("generated/project");
    file("generated/project/节点", "@!\nworker\n.worker\nworker!\n < |bad\n");
    execute_project("@!\n树\n.节点\n树!\n", "", false);
    expect_diagnostic("generated/project/节点:5:2");
    expect_diagnostic("generated/project/main:4:1");
    execute_project("@=\nbad\n.value\n |bad\n@!\nworker\n.worker\nworker!\n", "", false);
    expect_diagnostic("shared construction, type .value");
}

static void it_handles_target_bounds_refusal_but_not_index_decoder_failure(void) {
    execute_project("@!\nindex\n.value\n |20\nitems[]\n.worker\nseed\n.worker\nitems[index]!\n:\n seed!\n", "7:1:12\n", true);
    execute_project("@!\nindex\n.value\n |9999\nitems[]\n.worker\nseed\n.worker\nitems[index]!\n:\n seed!\n", "", false);
    expect_diagnostic("Index decoder failed");
}

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

static void it_selects_named_inputs_in_text_order_but_installs_in_declaration_order(void) {
    directory("generated/project");
    native_fixture("ordered","@<\na\n.value\n |1\nb\n.value\n a\n@>\nresult\n.value\n b\n@C\n");
    execute_project("@!\nordered\n.ordered\nfirst\n.value\n |1000\nsecond\n.value\n |1001\nitems[]\n.value\n |3\n |8\nordered!\n b < items[second]\n a < items[first]\n", "index=1001\nindex=1000\nordered=3,8,8\n",true);
}

static void it_reads_initial_and_last_results_without_reset_or_body_execution(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst\n >> result\nsecond!\n < result\nfirst!\n < |12\nfirst\n >> result\nsecond!\n < result\nfirst!\n", "7:1:0\n12:1:0\n13:2:0\n7:2:0\n",true);
}

static void it_writes_repeated_receivers_in_binding_text_order(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst!\n input < |4\n other > result\n output > result\nsecond!\n < result\n", "4:1:0\n5:1:0\n",true);
}

static void it_refuses_resume_without_preparing_the_interface(void) {
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nfirst%\n:\n second!\n", "7:1:10\n",true);
}

static void it_reports_failure_with_no_available_runtime_heap_or_root_frame(void) {
    file("generated/fault_allocator.h", "#include <stdlib.h>\n#define malloc(size) ((void)(size), (void *)0)\n");
    forced_header="generated/fault_allocator.h";
    execute_project("@!\nfirst\n.worker\nfirst!\n", "", false);
    forced_header=NULL;
    expect_diagnostic("Ama runtime failure: 2");
    expect_diagnostic("generated/project/main:1:1");
}

static void it_repeats_a_loop_and_returns_from_a_callee_to_the_right_caller(void) {
    directory("generated/project");
    native_fixture("decrement","@<\ninput\n.value\n@>\noutput\n.value\n@C\n");
    file("generated/project/returner","@!\nflag\n.value\n |1\nworker\n.worker\nflag\n?\n >\nworker!\n");
    execute_project("@!\ncounter\n.value\n |3\nstep\n.decrement\nearly\n.returner\nearly!\ncounter\n{\n step!\n  < counter\n  > counter\n}\n", "count=3\ncount=2\ncount=1\n",true);
}

static void it_reads_shared_namespaces_and_qualified_values_without_default_instances(void) {
    directory("generated/project");
    file("generated/project/numbers","@=\nfirst\n.value\n |5\nsecond\n.value\n |8\n");
    execute_project("@=\nshared\n.value\n |42\n@!\nworker\n.worker\nresult\n.value\nworker!\n < .numbers.first\n.numbers\n >> result\n >> result\nworker!\n < result\nworker!\n < .main.shared\n", "5:1:0\n8:2:0\n42:3:0\n",true);
}

void run(void) {
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
}
int main(void) { run(); return EXIT_SUCCESS; }
