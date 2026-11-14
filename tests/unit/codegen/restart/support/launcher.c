/** @file
 * @brief Observe cleanup instead of hidden execution after an unhandled standalone root yield.
 */
/** A completed native resource credit is consumed once when the launcher rejects
 * the yielded root. Reporting observes the count without running its continuation.
 */
static const char *report_worker_cleanup(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_DESTROY || strcmp(type->plan->type->module->source->logical_path, ".worker") != 0) { return NULL; }
    return " { assert(state->address==state); return printf(\"dropworker=%zu\\n\",state->calls)<0 ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
}

/** Standard launch enters once and diagnoses yield, then destroys the retained
 * state without secretly resuming the following source statement.
 */
static void it_rejects_unhandled_standard_root_yield_after_nonexecuting_cleanup(void) {
    native_hook_override = report_worker_cleanup;
    execute_project("@!\nprobe\n.worker\nprobe !\n < |10\n%\nprobe !\n < |999\n", "10:1:0\ndropworker=1\n", false);
    expect_diagnostic("Unhandled root yield");
    native_hook_override = NULL;
}
