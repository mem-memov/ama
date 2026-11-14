/** @file
 * @brief Separate fresh entry's interface defaults from persistent body fields and resume data.
 */
/** Entry replaces arguments and discards the previous position; resume preserves
 * arguments, while the same owned probe counter survives every invocation mode.
 */
static void it_restarts_entry_defaults_without_resetting_body_fields_or_resume_inputs(void) {
    directory("generated/project");
    file("generated/project/task", "@<\ninput\n.value\n |7\n@!\nprobe\n.worker\nprobe !\n < input\n%\nprobe !\n < input\n");
    execute_project("@!\ntask\n.task\nprobe\n.worker\ntask !\n < |10\ntask %\ntask !\n < |20\ntask !\ntask %\ntask %\n:\n probe !\n  < |99\n",
        "10:1:0\n10:2:0\n20:3:0\n7:4:0\n7:5:0\n99:1:10\n", true);
}
