/** @file
 * @brief Compare retained method/captured-state effects and complete resource cleanup against maintained expectations.
 */
/** Both toolchain modes run the same maintained Ama/native project; the byte
 * trace proves independent captured states and every expected destructor credit.
 */
static void run_constructor_project(bool sanitized) {
    char expected[4096];
    FILE *expectation = fopen("expected.txt", "rb");
    assert(expectation != NULL);
    read_stream(expectation, expected, sizeof(expected));
    int closed = fclose(expectation);
    assert(closed == 0);
    compile_and_run_object(".main", expected, NULL, sanitized);
}

/** Verified native execution must preserve two independent objects and their methods. */
static void it_runs_independent_captured_methods_and_complete_resource_cleanup(void) {
    run_constructor_project(false);
}

/** Instrument actual runtime/native code, not only the CLI assertion process. */
static void it_runs_captured_methods_under_target_address_and_undefined_behavior_sanitizers(void) {
    run_constructor_project(true);
}
