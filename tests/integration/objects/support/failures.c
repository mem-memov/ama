/** @file
 * @brief Verify native construction/body failures through the same real source-to-program pipeline.
 */
/** Accepted body failure bypasses its refusal alternative and cleans the two
 * constructed resources once, in reverse field order, preserving the primary diagnostic.
 */
static void it_reports_body_failure_without_running_an_acquisition_handler(void) {
    const char *expected = "drop=0\ndrop=18446744073709551615\n";
    compile_and_run_object("/test/fixture/v1.0.0/src/failure body", expected, "Counter exhausted", false);
    compile_and_run_object("/test/fixture/v1.0.0/src/failure body", expected, "Counter exhausted", true);
}

/** Invalid literal parses before resource acquisition. No default fallback,
 * body entry, destructor retry or later field construction is permitted.
 */
static void it_does_not_retry_failed_literal_construction(void) {
    compile_and_run_object("/test/fixture/v1.0.0/src/failure construction", "", "Not a decimal counter", false);
    compile_and_run_object("/test/fixture/v1.0.0/src/failure construction", "", "Not a decimal counter", true);
}
