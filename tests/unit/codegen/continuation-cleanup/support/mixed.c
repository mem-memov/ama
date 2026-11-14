/** @file
 * @brief Preserve each owned module's own position across mixed completed/yielded trees.
 */
/** A completed root can own a yielded child and a yielded root can own a
 * completed child. Snapshotting must preserve that exact per-node distinction.
 */
static void it_preserves_mixed_completed_and_yielded_owned_positions(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    file("generated/project/wrapper", "@>\nchild\n.task\n@!\nchild !\n");
    execute_project("@!\nsource\n.wrapper\ncopy\n.wrapper\nprobe\n.worker\nsource !\ncopy\n % source\ncopy %\n:\n probe !\n  < |99\ncopy\n child %\nsource\n child %\n",
        "1:1:0\n99:1:10\n2:2:0\n2:2:0\n", true);
    file("generated/project/task", "@!\nprobe\n.worker\nprobe !\n < |1\n");
    file("generated/project/wrapper", "@>\nchild\n.task\n@!\nprobe\n.worker\nchild !\n%\nprobe !\n < |4\n");
    execute_project("@!\nsource\n.wrapper\ncopy\n.wrapper\nprobe\n.worker\nsource !\ncopy\n % source\ncopy\n child %\n :\n  probe !\n   < |99\ncopy %\nsource %\n",
        "1:1:0\n99:1:10\n4:1:0\n4:1:0\n", true);
}
