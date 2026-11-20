/** @file
 * @brief Share one real two-capture project between compiled behavior and exhaustive generation-refusal checks.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Three declared ordinary instances use two keys and four specialization-scoped shared values. */
static const char *const captured_program = "@!\n/test/fixture/v1.0.0/src/factory\n .numbers\n first >> number\n/test/fixture/v1.0.0/src/factory\n .others\n first >> other\nfirst\n/test/fixture/v1.0.0/src/factory\n .numbers\nsecond\n/test/fixture/v1.0.0/src/factory\n .others\nagain\n/test/fixture/v1.0.0/src/factory\n .numbers\nfirst !\n < number\n > result\nsecond !\n < other\n > otherresult\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < result\notherprobe\n/test/fixture/v1.0.0/src/otherworker\notherprobe !\n < otherresult\nagain !\n < number\n > result\nprobe !\n < result\n";

/** Install actual source/static/native definitions before the common checked frontend consumes them. */
static void prepare_captured_definitions(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("other", "@C\n");
    native_fixture("otherworker", "@<\ninput\n/test/fixture/v1.0.0/src/other\n@>\noutput\n/test/fixture/v1.0.0/src/other\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/container\n .slot < /test/fixture/v1.0.0/src/value\n.others\n/test/fixture/v1.0.0/src/container\n .slot < /test/fixture/v1.0.0/src/other\n.unused\n/test/fixture/v1.0.0/src/container\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@=\nfirst\n.slot\n |17\nsecond\n.slot\n first\n@<\ninput\n.slot\n@>\noutput\n.slot\n input\n@!\noutput\n input\n");
}
