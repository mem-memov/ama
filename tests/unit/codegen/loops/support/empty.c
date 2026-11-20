/** @file
 * @brief Check true and false parsed-empty ordinary/header loops exactly once.
 */
/** Faithful native truth tracing distinguishes one-check semantics from a
 * callback that artificially changes the operand to make a broken loop terminate.
 */
static void it_checks_empty_named_and_header_loops_once_for_both_truth_values(void) {
    for (size_t truth = 0; truth < 2; truth++) {
        char source[256], expected[128];
        int length = snprintf(source, sizeof(source), "@!\ncondition\n/test/fixture/v1.0.0/src/value\n |%zu\nprobe\n/test/fixture/v1.0.0/src/worker\ncondition\n{\n # pending\n}\nprobe !\n < |99\n", truth);
        assert(length > 0 && (size_t)length < sizeof(source));
        length = snprintf(expected, sizeof(expected), "truth=%zu\n99:1:0\n", truth);
        assert(length > 0 && (size_t)length < sizeof(expected));
        execute_project(source, expected, true);
        length = snprintf(source, sizeof(source), "@>\ncondition\n/test/fixture/v1.0.0/src/value\n |%zu\n@!\ntrace\n/test/fixture/v1.0.0/src/worker\ntrace !\n < |10\n", truth);
        assert(length > 0 && (size_t)length < sizeof(source));
        file("generated/project/packages/test/fixture/v1.0.0/src/producer", source);
        length = snprintf(expected, sizeof(expected), "10:1:0\ntruth=%zu\n99:1:0\n", truth);
        assert(length > 0 && (size_t)length < sizeof(expected));
        execute_project("@!\nsource\n/test/fixture/v1.0.0/src/producer\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\n condition\n {\n  # pending\n }\nprobe !\n < |99\n", expected, true);
    }
}
