/** @file
 * @brief Execute a same-key nominal reference cycle and preserve per-instance Busy refusal and private native state.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** A reference to a running captured instance refuses before reentry, while another equal-key instance keeps independent private method state. */
static void it_refuses_busy_captured_self_references_without_merging_equal_key_instance_state(void) {
    directory("generated/project");
    file("generated/project/factory", "@...\n...numbers\n.busy\n ...slot < .value\n");
    file("generated/project/busy", "@<\ntag\n...slot\nnext&\n.factory\n ...numbers\n@!\nnext& !\n:\n probe\n .worker\n probe !\n");
    execute_project("@!\ntag\n.value\n |2\nfirst\n.factory\n ...numbers\nsecond\n.factory\n ...numbers\nfirst !\n tag < tag\n next& < &first\nsecond !\n tag < tag\n next& < &second\nfirst !\n tag < tag\n next& < &first\n", "7:1:9\n7:1:9\n7:2:9\n", true);
}
