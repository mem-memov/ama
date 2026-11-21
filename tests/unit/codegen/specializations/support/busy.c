/** @file
 * @brief Execute a same-key nominal reference cycle and preserve per-instance Busy refusal and private native state.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** A reference to a running captured instance refuses before reentry, while another equal-key instance keeps independent private method state. */
static void it_refuses_busy_captured_self_references_without_merging_equal_key_instance_state(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/busy\n .slot < /test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/busy", "@<\ntag\n.slot\nnext&\n/test/fixture/v1.0.0/src/factory\n .numbers\n@!\nnext& !\n:\n probe\n /test/fixture/v1.0.0/src/worker\n probe !\n");
    execute_project("@!\ntag\n/test/fixture/v1.0.0/src/value\n |2\nfirst\n/test/fixture/v1.0.0/src/factory\n .numbers\nsecond\n/test/fixture/v1.0.0/src/factory\n .numbers\nfirst !\n tag < tag\n next& < & first\nsecond !\n tag < tag\n next& < & second\nfirst !\n tag < tag\n next& < & first\n", "7:1:9\n7:1:9\n7:2:9\n", true);
}
