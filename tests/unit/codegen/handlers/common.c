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
    file("generated/project/branch", "@>\nlive\n.worker\nmissing&\n.worker\n@!\nresult\n.value\n |77\nprobe\n.worker\n");
    file("generated/project/middle", "@>\nbranch\n.branch\n@!\n");
    file("generated/project/box", "@>\nmiddle\n.middle\n@!\n");
}
#endif
