/** @file
 * @brief Separate owned-tree availability from non-owning references to running external actors.
 */
/** All owned sources are yielded/ready; references to the running helper are
 * outside the copied tree and therefore do not require acquisition or cloning.
 */
static void it_snapshots_available_owned_sources_with_busy_external_reference_targets(void) {
    prepare_availability_snapshot();
    native_hook_override = observe_snapshot_clone;
    execute_project("@!\nexecutor\n/test/fixture/v1.0.0/src/helper\nsource\n/test/fixture/v1.0.0/src/wrapper\nsource !\n < &executor\nexecutor !\n < &source\n",
        "1:1:0\ncloneworker\n", true);
    native_hook_override = NULL;
}

/** The available wrapper owns a task Waiting on the external helper. Reject
 * the whole source before any native clone, not after constructing a prefix.
 */
static void it_rejects_a_waiting_owned_descendant_before_any_clone_callback(void) {
    prepare_availability_snapshot();
    native_hook_override = observe_snapshot_clone;
    execute_project("@!\nexecutor\n/test/fixture/v1.0.0/src/helper\nsource\n/test/fixture/v1.0.0/src/wrapper\nsource !\n < &executor\n > child&\nchild& %\n",
        "1:1:0\n", false);
    expect_diagnostic("Ama runtime failure: 9");
    native_hook_override = NULL;
}
