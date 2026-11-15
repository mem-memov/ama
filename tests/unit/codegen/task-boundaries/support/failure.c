/** @file
 * @brief Retain native-only callbacks and prove context cleanup precedes failed host results.
 */
/** Restore the maintained typed native callback provider after generic fixture declaration generation. */
static void failure_native_body(NativeScaffold *scaffold) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, ".retainer") != 0) { continue; }
        char source[32768];
        boundary_source("project/retainer.c", source);
        file(type->body_path, source);
    }
}

/** Produce one technical error, not an ordinary cancellation result or a recoverable busy refusal. */
static bool failure_entry(FILE *body, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".crash") != 0) { return false; }
    int written = fputs("*status=ama_status_make(AMA_NATIVE_FAILURE,\"Forced task failure\",19); return;\n", body);
    assert(written != EOF);
    return true;
}

/** All waits detach before failed reaches the host; native acknowledgments outlive cleanup without running either continuation. */
static void it_detaches_every_wait_before_reporting_a_context_wide_technical_failure(void) {
    boundary_project();
    const char *names[] = {"waiting task", "retainer", "retainer.h", "retainer.c", "retainer.abi"};
    char input[256], output[256], source[32768], launcher[32768];
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        int length = snprintf(input, sizeof(input), "project/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        boundary_source(input, source);
        length = snprintf(output, sizeof(output), "generated/project/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(output));
        file(output, source);
    }
    native_fixture("crash", "@C\n");
    boundary_source("project/failure-host.c", launcher);
    boundary_source("project/technical-failure", source);
    host_launcher = launcher;
    native_entry_override = failure_entry;
    native_fixture_completion = failure_native_body;
    fprintf(stderr, "task boundary scenario: technical-failure\n");
    execute_project(source, "callbacks=closed,closed\nhost=failed\n", true);
    expect_diagnostic("Forced task failure");
    host_launcher = NULL;
    native_entry_override = NULL;
    native_fixture_completion = NULL;
}
