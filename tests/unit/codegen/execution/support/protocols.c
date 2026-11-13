/**
 * @file
 * @brief Verify generated execution rejects malformed entry and condition completion records.
 */

/** Native external output already completed is retained, while result actions and later calls stop. */
static void it_rejects_malformed_entry_completion_before_processing_call_results(void) {
    malformed_native_entry = true;
    execute_project("@!\nfirst\n.worker\nsecond\n.worker\nresult\n.value\nfirst!\n > result\nsecond!\n < result\n",
        "7:1:0\n", false);
    malformed_native_entry = false;
    expect_diagnostic("Ama runtime failure: 5: Malformed native status");
    expect_diagnostic("native entry, type .worker");
}

/** A callback-written true output must not select a body when its transport is malformed. */
static void it_rejects_malformed_condition_completion_without_executing_either_body(void) {
    malformed_condition = true;
    execute_project("@!\nvalue\n.value\n |1\nworker\n.worker\nvalue\n?\n worker!\n", "", false);
    malformed_condition = false;
    expect_diagnostic("Ama runtime failure: 5: Malformed native status");
}
