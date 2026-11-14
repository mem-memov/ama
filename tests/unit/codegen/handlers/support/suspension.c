/** @file
 * @brief Resume a handler after its abandoned chain has released every transient grant.
 */
/** Handler yield returns to an unrelated dynamic caller. Resume continues the
 * handler and outer statement, never a discarded selected sibling or ancestor.
 */
static void it_resumes_a_yielded_handler_without_restoring_abandoned_actions(void) {
    prepare_handler_tree();
    file("generated/project/controller", "@!\nobject\n.box\nprobe\n.worker\n"
        "object\n middle\n  branch\n   missing& !\n   :\n    probe !\n     < |1\n    %\n    probe !\n     < |2\n"
        "   live !\n    < |999\nprobe !\n < |3\nobject\n middle\n  branch\n   live !\n    < |4\n");
    execute_project("@!\ntask\n.controller\nprobe\n.worker\ntask !\nprobe !\n < |9\ntask %\nprobe !\n < |8\n",
        "1:1:7\n9:1:0\n2:2:7\n3:3:7\n4:1:7\n8:2:0\n", true);
}
