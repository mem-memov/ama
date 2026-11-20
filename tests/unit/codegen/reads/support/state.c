/** @file
 * @brief Observe full retained module value copying, cleared copied positions and unchanged referenced continuation state.
 */
/** Owned result transfer copies persistent native state while clearing saved positions; reference transfer leaves the source continuation intact. */
static void it_copies_retained_module_state_without_executing_resetting_or_copying_saved_positions(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |10\n%\nprobe !\n < |20\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/wrapper", "@>\ntask\n/test/fixture/v1.0.0/src/task\nactual&\n/test/fixture/v1.0.0/src/task\n &task\n@!\ntask !\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/wrapper\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\n > copy\n > actual&\ncopy %\n:\n probe !\n  < |99\nactual& %\nsource\n >> again\n >> retained&\nagain !\nretained& %\n:\n probe !\n  < |88\n", "10:1:0\n99:1:10\n20:2:0\n10:3:0\n88:2:10\n", true);
}
