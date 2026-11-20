/** @file
 * @brief Observe repeated immediate-caller returns and Waiting versus Yielded eligibility.
 */
/** Every task segment sees its actual waiting parent as Busy. Each yield makes
 * the task available for the parent's next resume without copying a caller chain.
 */
static void it_returns_yields_to_waiting_callers_and_releases_callees_for_resume(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@<\nparent&\n/test/fixture/v1.0.0/src/outer\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "parent& !\n:\n probe !\n  < |1\n%\nparent& !\n:\n probe !\n  < |2\n%\nparent& !\n:\n probe !\n  < |3\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/outer", "@!\ntask\n/test/fixture/v1.0.0/src/task\nprobe\n/test/fixture/v1.0.0/src/worker\ntask !\n < &\nprobe !\n < |10\ntask %\nprobe !\n < |20\ntask %\nprobe !\n < |30\n");
    execute_project("@!\nouter\n/test/fixture/v1.0.0/src/outer\nprobe\n/test/fixture/v1.0.0/src/worker\nouter !\nprobe !\n < |40\n",
        "1:1:9\n10:1:0\n2:2:9\n20:2:0\n3:3:9\n30:3:0\n40:1:0\n", true);
}

/** Completion clears the module's saved position. An additional unhandled
 * resume refuses instead of replaying the last segment or inventing an entry.
 */
static void it_refuses_resume_after_the_last_saved_segment_completes(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\n%\nprobe !\n < |1\n");
    execute_project("@!\ntask\n/test/fixture/v1.0.0/src/task\ntask !\ntask %\ntask %\n", "1:1:0\n", false);
    expect_diagnostic("Ama runtime failure: 10");
}
