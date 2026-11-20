/** @file
 * @brief Preserve every owned saved position only for explicit copies while clearing transient service histories.
 */
/** Ordinary copying clears both root/child positions. Explicit snapshots retain
 * each independent position, preserve native values and reset refusal history.
 */
static void it_copies_owned_tree_positions_explicitly_and_clears_ordinary_positions_and_histories(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nself&\n/test/fixture/v1.0.0/src/task\n &\nprobe\n/test/fixture/v1.0.0/src/worker\nself& !\n:\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/wrapper", "@>\nchild\n/test/fixture/v1.0.0/src/task\n@!\nself&\n/test/fixture/v1.0.0/src/wrapper\n &\nprobe\n/test/fixture/v1.0.0/src/worker\nself& !\n:\nchild !\n%\nchild %\nprobe !\n < |4\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/wrapper\nplain\n/test/fixture/v1.0.0/src/wrapper\ncopy\n/test/fixture/v1.0.0/src/wrapper\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\nplain\n source\n"
        "plain\n child %\n :\n  probe !\n   < |99\nplain %\n:\n probe !\n  < |88\ncopy\n % source\ncopy %\nsource %\n",
        "1:1:9\n99:1:10\n88:2:10\n2:2:0\n4:1:0\n2:2:9\n4:1:9\n", true);
}
