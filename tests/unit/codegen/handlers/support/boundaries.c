/** @file
 * @brief Distinguish link-local alternatives from ancestor handlers and method-local initiating calls.
 */
/** An unhandled descendant refusal bypasses handlers on both enclosing links
 * and terminates instead of running the following caller statement.
 */
static void it_does_not_offer_a_descendant_refusal_to_ancestor_handlers(void) {
    prepare_handler_tree();
    execute_project("@!\nobject\n.box\nprobe\n.worker\nobject\n middle\n  branch\n   missing& !\n"
        "  :\n   probe !\n    < |111\n :\n  probe !\n   < |222\n:\n probe !\n  < |333\n"
        "probe !\n < |444\n", "", false);
    expect_diagnostic("Ama runtime failure: 7");
}

/** A selected method's handled inner call owns a different initiating operation.
 * It must preserve the outer caller's pending result actions and ancestor grants.
 */
static void it_keeps_unrelated_outer_callers_when_a_method_handles_its_own_refusal(void) {
    prepare_handler_tree();
    file("generated/project/method", "@>\noutput\n.value\n |7\n@!\ninner\n.box\nprobe\n.worker\n"
        "inner\n middle\n  branch\n   missing& !\n   :\n    probe !\n     < |11\n");
    file("generated/project/holder", "@>\nmethod\n.method\nsibling\n.worker\n@!\n");
    execute_project("@!\nobject\n.holder\nprobe\n.worker\nobject\n method !\n  > result\n"
        " sibling !\n  < result\nprobe !\n < result\n", "11:1:7\n7:1:0\n7:1:0\n", true);
}

/** A body failure happens after acquisition and can never become a refused link,
 * even when alternatives are attached at every nesting level.
 */
static void it_keeps_selected_body_failures_technical_at_every_handler_level(void) {
    prepare_handler_tree();
    execute_project("@!\nobject\n.box\nprobe\n.worker\nobject\n middle\n  branch\n   live !\n    < |500\n"
        "   :\n    probe !\n     < |111\n  :\n   probe !\n    < |222\n :\n  probe !\n   < |333\n"
        ":\n probe !\n  < |444\n", "native-entered\n", false);
    expect_diagnostic("Body failed");
}
