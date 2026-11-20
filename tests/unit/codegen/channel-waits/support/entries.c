/** @file
 * @brief Emit controlled actor failure/cancellation and terminal-outcome checks over ordinary typed fields.
 * Native probes do not execute Ama and inherit the existing independent port/codec fixtures.
 */
/** Keep backpressure, cancellation and technical context failure observably separate. */
static bool wait_actor_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    const char *body = NULL;
    if (strcmp(path, "/test/fixture/v1.0.0/src/ready") == 0) { body = "puts(\"ready\");\n"; }
    else if (strcmp(path, "/test/fixture/v1.0.0/src/maybefail") == 0) {
        body = "bool test_channel_should_fail(void); if(test_channel_should_fail()) { *status=ama_status_make(AMA_NATIVE_FAILURE,\"Channel actor failed\",20); return; }\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/requestcancel") == 0) {
        body = "bool test_channel_should_cancel(void); results->AMA_OUTPUT(0)->payload->value=test_channel_should_cancel() ? 1 : 0;\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/report") == 0) {
        body = "unsigned test_channel_expected_outcome(void); bool test_channel_expected_failure(void);\n"
            "unsigned outcome=parameters->AMA_INPUT(0)->payload->terminal; bool failed=parameters->AMA_INPUT(1)->payload->failed;\n"
            "if(outcome!=test_channel_expected_outcome() || failed!=test_channel_expected_failure()) { *status=ama_status_make(AMA_NATIVE_FAILURE,NULL,0); return; }\n"
            "puts(outcome==1 ? \"cancelled\" : failed ? \"producer failed\" : \"producer complete\");\n";
    }
    if (body == NULL) { return channel_port_entry(output, type); }
    int written = fputs(body, output);
    assert(written != EOF);
    return true;
}
