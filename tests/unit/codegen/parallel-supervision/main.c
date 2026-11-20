/** @file
 * @brief Compile the maintained actor example with simultaneous executors and explicit independent supervisor policies.
 * Exact producer metadata supplies native bridges; generated implementation is never inspected.
 */
#include "../channel-waits/support/install.c"

/** Capture immutable native probe configuration during serialized construction, before either worker starts. */
static const char *supervision_hook(const ScaffoldType *type, AbiOperation operation) {
    const char *path = type->plan->type->module->source->logical_path;
    if ((strcmp(path, "/test/fixture/v1.0.0/src/role") == 0 || strcmp(path, "/test/fixture/v1.0.0/src/maybefail") == 0) && operation == ABI_DEFAULT) {
        return " { bool test_supervision_factory_role(void); *destination=(AMA_NATIVE(State)){.address=destination,.number=test_supervision_factory_role() ? 1 : 0}; return ama_status_success(); }\n";
    }
    return channel_port_hook(type, operation);
}
/** Runtime role comes from its own constructed payload, never a mutable current-context global. */
static bool supervision_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    const char *body = NULL;
    if (strcmp(path, "/test/fixture/v1.0.0/src/role") == 0) { body = "results->AMA_OUTPUT(0)->payload->value=state->number;\n"; }
    else if (strcmp(path, "/test/fixture/v1.0.0/src/maybefail") == 0) {
        body = "bool test_supervision_failure(bool); if(test_supervision_failure(state->number!=0)) { *status=ama_status_make(AMA_NATIVE_FAILURE,\"Supervised receiver failed\",26); return; }\n";
    }
    if (body == NULL) { return wait_actor_entry(output, type); }
    int written = fputs(body, output);
    assert(written != EOF);
    return true;
}
/** Identical receiver failure can preserve peer execution or explicitly cancel it, with no implicit contagion. */
static void it_supervises_parallel_channel_actors_with_explicit_continue_and_cancel_policies(void) {
    install_wait_fixture();
    char source[16384], consumer[16384];
    read_fixture("project/main", source, sizeof(source));
    FILE *input = fopen(TEST_SDK "/../examples/parallel/host.c", "rb");
    assert(input != NULL);
    size_t length = fread(consumer, 1, sizeof(consumer)-1, input);
    assert(!ferror(input) && length < sizeof(consumer)-1);
    int closed = fclose(input);
    assert(closed == 0);
    consumer[length] = 0;
    host_launcher = consumer;
    native_entry_override = supervision_entry;
    native_hook_override = supervision_hook;
    native_fixture_completion = complete_wait_native;
    execute_project(source, "parallel supervision accepted\n", true);
    expect_diagnostic("Supervised receiver failed");
    native_fixture_completion = NULL;
    native_hook_override = NULL;
    native_entry_override = NULL;
    host_launcher = NULL;
}
/** Run the actual generated actor/supervisor composition without timing-based overlap assumptions. */
void run(void) { it_supervises_parallel_channel_actors_with_explicit_continue_and_cancel_policies(); }
/** Successful exit includes release-effective ownership assertions in the generated consumer. */
int main(void) { run(); return EXIT_SUCCESS; }
