/** @file
 * @brief Observe ancestor Busy protection and release before a selected link's handler.
 */
/** A returned self reference cannot reenter its retained parent. After its own
 * handler the same parent is available to a new root chain, proving release.
 */
static void it_protects_the_parent_until_a_refused_link_abandons_the_chain(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/holder", "@>\nself&\n/test/fixture/v1.0.0/src/holder\n &\nmethod\n/test/fixture/v1.0.0/src/worker\n@!\n");
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/holder\nprobe\n/test/fixture/v1.0.0/src/worker\nobject\n self& !\n :\n  probe !\n   < |55\n method !\n  < |999\nobject\n method !\n  < |66\n", "55:1:9\n66:1:9\n", true);
}
