/** @file
 * @brief Observe full retained module value copying, cleared copied positions and unchanged referenced continuation state.
 */
/** Owned result transfer copies persistent native state while clearing saved positions; reference transfer leaves the source continuation intact. */
static void it_copies_retained_module_state_without_executing_resetting_or_copying_saved_positions(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe !\n < |10\n%\nprobe !\n < |20\n");
    file("generated/project/wrapper", "@>\ntask\n.task\nactual&\n.task\n &task\n@!\ntask !\n");
    execute_project("@!\nsource\n.wrapper\nprobe\n.worker\nsource !\n > copy\n > actual&\ncopy %\n:\n probe !\n  < |99\nactual& %\nsource\n >> again\n >> retained&\nagain !\nretained& %\n:\n probe !\n  < |88\n", "10:1:0\n99:1:10\n20:2:0\n10:3:0\n88:2:10\n", true);
}
