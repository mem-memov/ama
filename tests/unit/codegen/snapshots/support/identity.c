/** @file
 * @brief Reject continuation self-copy through a reference alias before allocating or changing native state.
 */
/** Distinct source spellings still denote the same actual instance. Terminal
 * cleanup must see its original yielded payload and no clone preparation effects.
 */
static void it_rejects_continuation_copy_to_the_same_actual_identity_through_an_alias(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n%\n");
    native_hook_override = observe_snapshot_worker;
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\nalias&\n/test/fixture/v1.0.0/src/task\n & task\ntask !\ntask\n % alias&\n",
        "1:1:0\ndropworker=1\n", false);
    expect_diagnostic("Ama runtime failure: 5");
    native_hook_override = NULL;
}
