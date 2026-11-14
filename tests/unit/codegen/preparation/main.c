/** @file
 * @brief Compile declaration-ordered defaults, caller/callee self sources and technical preparation failures through the real frontend and runtime.
 */
#include "../execution/common.c"

/** Native observation sees installed parameters and result defaults, independently of named binding order and omitted placeholders. */
static void it_installs_parameters_then_results_in_declaration_order_with_omitted_defaults(void) {
    directory("generated/project");
    native_fixture("ordered", "@<\nfirst\n.value\n |2\nsecond\n.value\n first\n@>\noutput\n.value\n second\n@C\n");
    execute_project("@!\ncall\n.ordered\nvalue\n.value\n |12\ncall !\n second < value\n first < |4\n output > result\ncall !\n < |9\n > result\ncall !\n <\n > result\n",
        "ordered=4,12,12\nordered=9,9,9\nordered=2,2,2\n", true);
}

/** Declare one reference-only caller link and one self result without hiding handles in native State. */
static void prepare_self_relay(void) {
    directory("generated/project");
    file("generated/project/relay", "@<\nparent&\n.main\n@>\nme&\n.relay\n &\n@!\nprobe\n.worker\nparent& !\n:\n probe !\n");
}

/** Supplied & identifies the Waiting caller without acquiring it; a default & returns the same callee so its body state survives invocation through that reference. */
static void it_distinguishes_caller_self_inputs_from_callee_self_defaults(void) {
    prepare_self_relay();
    execute_project("@!\nrelay\n.relay\nrelay !\n < &\n > returned&\nreturned& !\n < &\n > returned&\n", "7:1:9\n7:2:9\n", true);
}

/** Ordinary cloning remaps a stored self result to the destination; a retained read exposes it before entry can recompute the default. */
static void it_remaps_compiled_self_results_when_copying_an_instance_before_retained_read(void) {
    prepare_self_relay();
    execute_project("@!\nrelay\n.relay\nrelay !\n < &\n > original&\ncopy\n relay\ncopy\n >> copied&\ncopied& !\n < &\noriginal& !\n < &\n", "7:1:9\n7:2:9\n7:2:9\n", true);
}

/** A later constructor failure stops body/result processing and is not handled by the call's refusal alternative. */
static void it_stops_before_body_and_refusal_handler_when_a_later_parameter_constructor_fails(void) {
    directory("generated/project");
    native_fixture("ordered", "@<\nfirst\n.value\n |2\nsecond\n.value\n first\n@>\noutput\n.value\n second\n@C\n");
    execute_project("@!\ncall\n.ordered\nprobe\n.worker\ncall !\n first < |4\n second < |bad\n:\n probe !\n", "", false);
    expect_diagnostic("Not a decimal value");
    expect_diagnostic("binding, type .value");
}

/** Run positive order/identity cases and the independently located technical failure. */
void run(void) {
    it_installs_parameters_then_results_in_declaration_order_with_omitted_defaults();
    it_distinguishes_caller_self_inputs_from_callee_self_defaults();
    it_remaps_compiled_self_results_when_copying_an_instance_before_retained_read();
    it_stops_before_body_and_refusal_handler_when_a_later_parameter_constructor_fails();
}

/** Return success only after every compiled preparation case completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
