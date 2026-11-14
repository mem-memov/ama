/** @file
 * @brief Preserve every owned saved position only for explicit copies while clearing transient service histories.
 */
/** Ordinary copying clears both root/child positions. Explicit snapshots retain
 * each independent position, preserve native values and reset refusal history.
 */
static void it_copies_owned_tree_positions_explicitly_and_clears_ordinary_positions_and_histories(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nself&\n.task\n &\nprobe\n.worker\nself& !\n:\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    file("generated/project/wrapper", "@>\nchild\n.task\n@!\nself&\n.wrapper\n &\nprobe\n.worker\nself& !\n:\nchild !\n%\nchild %\nprobe !\n < |4\n");
    execute_project("@!\nsource\n.wrapper\nplain\n.wrapper\ncopy\n.wrapper\nprobe\n.worker\nsource !\nplain\n source\n"
        "plain\n child %\n :\n  probe !\n   < |99\nplain %\n:\n probe !\n  < |88\ncopy\n % source\ncopy %\nsource %\n",
        "1:1:9\n99:1:10\n88:2:10\n2:2:0\n4:1:0\n2:2:9\n4:1:9\n", true);
}
