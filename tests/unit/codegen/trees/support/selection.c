/** @file
 * @brief Distinguish returned reference identity and caller-local indexed selection.
 */
/** A returned reference and an owned-list element are invoked in place; the
 * caller's index is used even when the parent declares a different same-name index.
 */
static void it_follows_returned_references_and_uses_caller_indices(void) {
    directory("generated/project");
    file("generated/project/holder", "@>\nmethod&\n.worker\nmethods[]\n.worker\n .template.value\n .template.value\n@!\nactual\n.worker\nindex\n.value\n |1\nmethod&\n &actual\n");
    file("generated/project/template", "@=\nvalue\n.worker\n");
    execute_project("@!\nobject\n.holder\nindex\n.value\n |0\nobject !\n method& !\n  < |31\n methods[index] !\n  < |41\nobject\n method& !\n  < |32\n methods[index] !\n  < |42\n", "31:1:0\n41:1:0\n32:2:0\n42:2:0\n", true);
}

/** A native index decoder error terminates before selecting a method; the
 * selected link's refusal handler must not turn a technical failure into success.
 */
static void it_keeps_selected_index_protocol_failures_technical(void) {
    directory("generated/project");
    file("generated/project/holder", "@>\nmethods[]\n.worker\n@!\n");
    execute_project("@!\nobject\n.holder\nindex\n.value\n |9999\nprobe\n.worker\nobject\n methods[index] !\n :\n  probe !\n", "", false);
    expect_diagnostic("Index decoder failed");
}
