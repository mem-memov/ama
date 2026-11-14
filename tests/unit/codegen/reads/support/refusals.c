/** @file
 * @brief Keep existing receivers unchanged when a retained target is busy or an old owned child's checked reference is stale.
 */
/** Reading the currently executing root must refuse before any result receiver is selected or replaced. */
static void it_refuses_a_busy_retained_target_before_changing_its_receiver(void) {
    execute_project("@>\noutput\n.value\n |7\n@!\nself&\n.main\n &\nreceiver\n.value\n |42\nprobe\n.worker\nself&\n >> receiver\n:\n probe !\n  < receiver\n", "42:1:9\n", true);
}

/** Replacing an owned parent invalidates its old child while preserving the parent's identity; the stale child reference refuses a later read before receiver mutation. */
static void it_refuses_a_stale_retained_target_before_changing_its_receiver(void) {
    directory("generated/project");
    file("generated/project/record", "@>\nvalue\n.value\n |7\n@!\n");
    file("generated/project/box", "@>\nchild&\n.record\n@!\nelement\n.record\nchild&\n &element\n");
    execute_project("@!\nbox\n.box\nreplacement\n.box\nreceiver\n.value\n |42\nprobe\n.worker\nbox !\n > saved&\nbox\n replacement\nsaved&\n >> receiver\n:\n probe !\n  < receiver\n", "42:1:8\n", true);
}
