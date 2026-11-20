/**
 * @file
 * @brief Verify generated execution rejects malformed entry and condition completion records.
 */

/** Native external output already completed is retained, while result actions and later calls stop. */
static void it_rejects_malformed_entry_completion_before_processing_call_results(void) {
    malformed_native_entry = true;
    execute_project("@!\nfirst\n/test/fixture/v1.0.0/src/worker\nsecond\n/test/fixture/v1.0.0/src/worker\nresult\n/test/fixture/v1.0.0/src/value\nfirst !\n > result\nsecond !\n < result\n",
        "7:1:0\n", false);
    malformed_native_entry = false;
    expect_diagnostic("Ama runtime failure: 5: Malformed native status");
    expect_diagnostic("native entry, type /test/fixture/v1.0.0/src/worker");
}

/** A callback-written true output must not select a body when its transport is malformed. */
static void it_rejects_malformed_condition_completion_without_executing_either_body(void) {
    malformed_condition = true;
    execute_project("@!\nvalue\n/test/fixture/v1.0.0/src/value\n |1\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n?\n worker !\n", "", false);
    malformed_condition = false;
    expect_diagnostic("Ama runtime failure: 5: Malformed native status");
}

/** Every native payload construction mode and destruction must validate transport before entering or continuing Ama execution. */
static void it_checks_malformed_lifecycle_statuses_in_actual_generated_execution(void) {
    const char *sources[] = {
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n",
        "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\n |3\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\n |3\ncopy\n/test/fixture/v1.0.0/src/value\n value\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n"
    };
    for (int operation = ABI_DEFAULT; operation <= ABI_CLONE; operation++) {
        malformed_lifecycle = operation;
        malformed_lifecycle_module = operation == ABI_DESTROY ? "/test/fixture/v1.0.0/src/worker" : "/test/fixture/v1.0.0/src/value";
        execute_project(sources[operation], operation == ABI_DESTROY ? "7:1:0\n" : "", false);
        expect_diagnostic("Malformed native status");
    }
    malformed_lifecycle = -1;
    malformed_lifecycle_module = NULL;
}
