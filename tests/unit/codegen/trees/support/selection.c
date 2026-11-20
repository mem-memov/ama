/** @file
 * @brief Distinguish returned reference identity and caller-local indexed selection.
 */
/** A returned reference and an owned-list element are invoked in place; the
 * caller's index is used even when the parent declares a different same-name index.
 */
static void it_follows_returned_references_and_uses_caller_indices(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/holder", "@>\nmethod&\n/test/fixture/v1.0.0/src/worker\nmethods[]\n/test/fixture/v1.0.0/src/worker\n /test/fixture/v1.0.0/src/template/value\n /test/fixture/v1.0.0/src/template/value\n@!\nactual\n/test/fixture/v1.0.0/src/worker\nindex\n/test/fixture/v1.0.0/src/value\n |1\nmethod&\n &actual\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/template", "@=\nvalue\n/test/fixture/v1.0.0/src/worker\n");
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/holder\nindex\n/test/fixture/v1.0.0/src/value\n |0\nobject !\n method& !\n  < |31\n methods[index] !\n  < |41\nobject\n method& !\n  < |32\n methods[index] !\n  < |42\n", "31:1:0\n41:1:0\n32:2:0\n42:2:0\n", true);
}

/** A native index decoder error terminates before selecting a method; the
 * selected link's refusal handler must not turn a technical failure into success.
 */
static void it_keeps_selected_index_protocol_failures_technical(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/holder", "@>\nmethods[]\n/test/fixture/v1.0.0/src/worker\n@!\n");
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/holder\nindex\n/test/fixture/v1.0.0/src/value\n |9999\nprobe\n/test/fixture/v1.0.0/src/worker\nobject\n methods[index] !\n :\n  probe !\n", "", false);
    expect_diagnostic("Index decoder failed");
}
