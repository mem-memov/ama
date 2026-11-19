/** @file
 * @brief Pass a determined partial Template through another static producer and execute its concrete result.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Residual formals and fixed captures survive family passing; filling the residual explicitly produces one ordinary executable Type. */
static void it_executes_a_partial_family_completed_by_another_static_producer(void) {
    directory("generated/project");
    native_fixture("other", "@C\n");
    native_fixture("otherworker", "@<\ninput\n.other\n@>\noutput\n.other\n@C\n");
    file("generated/project/pair", "@<\nleft\n...key\nright\n...value\n@>\noutput\n...value\n right\n@!\noutput\n right\n");
    file("generated/project/partial", "@...\n...for value\n.pair\n ...key < .value\n");
    file("generated/project/complete", "@...\n...result\n...family\n ...value < .other\n");
    file("generated/project/assembly", "@...\n.partial\n ...for value > ...partial\n.complete\n ...family < ...partial\n ...result > ...ready\n");
    execute_project("@!\nkey\n.value\n |3\nvalue\n.other\n |41\npair\n.assembly\n ...ready\npair !\n left < key\n right < value\n output > result\nprobe\n.otherworker\nprobe !\n < result\n", "41:1:0\n", true);
}
