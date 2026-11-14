/** @file
 * @brief Return yielded selected bodies to their initiating chain and resume them in place.
 */
/** Both reference and owned continuations keep their own positions while parent
 * actions proceed normally after each selected yield.
 */
static void it_resumes_returned_continuations_without_suspending_the_chain(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe !\n < |10\n%\nprobe !\n < |20\n%\nprobe !\n < |30\n");
    file("generated/project/producer", "@>\nowned\n.task\nreference&\n.task\n@!\nactual\n.task\nowned !\nactual !\nreference&\n &actual\n");
    execute_project("@!\nsource\n.producer\nprobe\n.worker\nsource !\n owned %\n reference& %\nprobe !\n < |99\nsource\n owned %\n reference& %\n", "10:1:0\n10:1:0\n20:2:0\n20:2:0\n99:1:0\n30:3:0\n30:3:0\n", true);
}
