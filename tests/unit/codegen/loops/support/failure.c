/** @file
 * @brief Reject failed terminal conversion before body or acquisition-refusal handlers.
 */
/** Native failure can write true to scratch, but no successful truth is published. */
static void it_keeps_terminal_conversion_failure_outside_refusal_handlers(void) {
    directory("generated/project");
    file("generated/project/producer", "@>\ncondition\n.value\n |500\n@!\n");
    execute_project("@!\nsource\n.producer\nprobe\n.worker\nsource !\n condition\n {\n  probe !\n   < |999\n }\n:\n probe !\n  < |888\n",
        "truth=500\n", false);
    expect_diagnostic("Condition conversion failed");
}
