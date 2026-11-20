/** @file
 * @brief Pass a determined partial Template through another static producer and execute its concrete result.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Residual formals and fixed captures survive family passing; filling the residual explicitly produces one ordinary executable Type. */
static void it_executes_a_partial_family_completed_by_another_static_producer(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("other", "@C\n");
    native_fixture("otherworker", "@<\ninput\n/test/fixture/v1.0.0/src/other\n@>\noutput\n/test/fixture/v1.0.0/src/other\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/pair", "@<\nleft\n.key\nright\n.value\n@>\noutput\n.value\n right\n@!\noutput\n right\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/partial", "@...\n.for value\n/test/fixture/v1.0.0/src/pair\n .key < /test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/complete", "@...\n.result\n.family\n .value < /test/fixture/v1.0.0/src/other\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly", "@...\n/test/fixture/v1.0.0/src/partial\n .for value > .partial\n/test/fixture/v1.0.0/src/complete\n .family < .partial\n .result > .ready\n");
    execute_project("@!\nkey\n/test/fixture/v1.0.0/src/value\n |3\nvalue\n/test/fixture/v1.0.0/src/other\n |41\npair\n/test/fixture/v1.0.0/src/assembly\n .ready\npair !\n left < key\n right < value\n output > result\nprobe\n/test/fixture/v1.0.0/src/otherworker\nprobe !\n < result\n", "41:1:0\n", true);
}
