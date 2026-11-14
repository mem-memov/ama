/** @file
 * @brief Preserve committed outputs/method state while abandoning every unfinished ancestor action.
 */
/** Deep refusal releases the complete path before a fresh handler-root call;
 * completed output and method effects survive while all pending siblings skip.
 */
static void it_unwinds_deep_handlers_in_caller_scope_and_preserves_prior_progress(void) {
    prepare_handler_tree();
    execute_project("@!\nobject\n.box\nresult\n.value\n |41\nprobe\n.worker\n"
        "object\n middle\n  branch\n   live !\n    < |10\n    > result\n"
        "   missing& !\n   :\n    probe !\n     < result\n"
        "    object\n     middle\n      branch\n       live !\n        < |20\n"
        "   live !\n    < |999\n  branch\n   live !\n    < |888\n"
        " middle\n  branch\n   live !\n    < |777\n"
        "probe !\n < result\nobject\n middle\n  branch\n   live !\n    < |30\n",
        "10:1:0\n11:1:7\n20:2:7\n11:2:7\n30:3:7\n", true);
}

/** Empty attached handlers handle refusal and continue after the complete root,
 * with no implicit retry or pending sibling execution.
 */
static void it_accepts_empty_handlers_after_releasing_the_complete_tree(void) {
    prepare_handler_tree();
    execute_project("@!\nobject\n.box\nobject\n middle\n  branch\n   missing& !\n   :\n"
        "   live !\n    < |999\nobject\n middle\n  branch\n   live !\n    < |10\n", "10:1:7\n", true);
}
