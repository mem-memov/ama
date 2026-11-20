/** @file
 * @brief Keep existing receivers unchanged when a retained target is busy or an old owned child's checked reference is stale.
 */
/** Reading the currently executing root must refuse before any result receiver is selected or replaced. */
static void it_refuses_a_busy_retained_target_before_changing_its_receiver(void) {
    execute_project("@>\noutput\n/test/fixture/v1.0.0/src/value\n |7\n@!\nself&\n/test/fixture/v1.0.0/src/main\n &\nreceiver\n/test/fixture/v1.0.0/src/value\n |42\nprobe\n/test/fixture/v1.0.0/src/worker\nself&\n >> receiver\n:\n probe !\n  < receiver\n", "42:1:9\n", true);
}

/** Replacing an owned parent invalidates its old child while preserving the parent's identity; the stale child reference refuses a later read before receiver mutation. */
static void it_refuses_a_stale_retained_target_before_changing_its_receiver(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/record", "@>\nvalue\n/test/fixture/v1.0.0/src/value\n |7\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@>\nchild&\n/test/fixture/v1.0.0/src/record\n@!\nelement\n/test/fixture/v1.0.0/src/record\nchild&\n &element\n");
    execute_project("@!\nbox\n/test/fixture/v1.0.0/src/box\nreplacement\n/test/fixture/v1.0.0/src/box\nreceiver\n/test/fixture/v1.0.0/src/value\n |42\nprobe\n/test/fixture/v1.0.0/src/worker\nbox !\n > saved&\nbox\n replacement\nsaved&\n >> receiver\n:\n probe !\n  < receiver\n", "42:1:8\n", true);
}
