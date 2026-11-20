/** @file
 * @brief Preserve each owned module's own position across mixed completed/yielded trees.
 */
/** A completed root can own a yielded child and a yielded root can own a
 * completed child. Snapshotting must preserve that exact per-node distinction.
 */
static void it_preserves_mixed_completed_and_yielded_owned_positions(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/wrapper", "@>\nchild\n/test/fixture/v1.0.0/src/task\n@!\nchild !\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/wrapper\ncopy\n/test/fixture/v1.0.0/src/wrapper\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\ncopy\n % source\ncopy %\n:\n probe !\n  < |99\ncopy\n child %\nsource\n child %\n",
        "1:1:0\n99:1:10\n2:2:0\n2:2:0\n", true);
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/wrapper", "@>\nchild\n/test/fixture/v1.0.0/src/task\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\nchild !\n%\nprobe !\n < |4\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/wrapper\ncopy\n/test/fixture/v1.0.0/src/wrapper\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\ncopy\n % source\ncopy\n child %\n :\n  probe !\n   < |99\ncopy %\nsource %\n",
        "1:1:0\n99:1:10\n4:1:0\n4:1:0\n", true);
}
