/** @file
 * @brief Build a three-owner method tree with caller-name collisions and an empty returned reference.
 */
#ifndef AMA_HANDLER_TEST_COMMON
#define AMA_HANDLER_TEST_COMMON
#include "../execution/common.c"

/** The branch's private result/probe names deliberately collide with caller
 * operands; selectors still resolve only its two declared returned methods.
 */
static void prepare_handler_tree(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/branch", "@>\nlive\n/test/fixture/v1.0.0/src/worker\nmissing&\n/test/fixture/v1.0.0/src/worker\n@!\nresult\n/test/fixture/v1.0.0/src/value\n |77\nprobe\n/test/fixture/v1.0.0/src/worker\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/middle", "@>\nbranch\n/test/fixture/v1.0.0/src/branch\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@>\nmiddle\n/test/fixture/v1.0.0/src/middle\n@!\n");
}
#endif
