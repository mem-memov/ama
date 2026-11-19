/** @file
 * @brief Share one real two-capture project between compiled behavior and exhaustive generation-refusal checks.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Three declared ordinary instances use two keys and four specialization-scoped shared values. */
static const char *const captured_program = "@!\n.factory\n ...numbers\n first >> number\n.factory\n ...others\n first >> other\nfirst\n.factory\n ...numbers\nsecond\n.factory\n ...others\nagain\n.factory\n ...numbers\nfirst !\n < number\n > result\nsecond !\n < other\n > otherresult\nprobe\n.worker\nprobe !\n < result\notherprobe\n.otherworker\notherprobe !\n < otherresult\nagain !\n < number\n > result\nprobe !\n < result\n";

/** Install actual source/static/native definitions before the common checked frontend consumes them. */
static void prepare_captured_definitions(void) {
    directory("generated/project");
    native_fixture("other", "@C\n");
    native_fixture("otherworker", "@<\ninput\n.other\n@>\noutput\n.other\n@C\n");
    file("generated/project/factory", "@...\n...numbers\n.container\n ...slot < .value\n...others\n.container\n ...slot < .other\n...unused\n.container\n");
    file("generated/project/container", "@=\nfirst\n...slot\n |17\nsecond\n...slot\n first\n@<\ninput\n...slot\n@>\noutput\n...slot\n input\n@!\noutput\n input\n");
}
