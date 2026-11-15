/** @file
 * @brief Compile a maintained parallel host through actual typed native bridges and observe its assertions.
 * The consumer is assembled from maintained fragments; generated implementation is never inspected.
 */
#include "../execution/common.c"

/** Read exact maintained fixture bytes into a bounded caller-owned assembly buffer. */
static void append_fixture(const char *path, char *buffer, size_t capacity) {
    size_t used = strlen(buffer);
    FILE *input = fopen(path, "rb");
    assert(input != NULL && used + 1 < capacity);
    size_t count = fread(buffer + used, 1, capacity - used - 1, input);
    assert(!ferror(input) && count < capacity - used - 1);
    int closed = fclose(input);
    assert(closed == 0);
    buffer[used + count] = 0;
}

/** Borrow a service synchronously; only its independently owned native endpoint escapes. */
static bool parallel_native_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    int written;
    if (strcmp(path, ".bridge") == 0) {
        written = fputs("AmaStatus test_parallel_service(const ama_t_0a13545c236dc688542e3824b23bf3848f38fdb81d6b4c289732f8a82eaa888f *,void *);\n"
            "*status=parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage,test_parallel_service,NULL);\n"
            "if(status->code!=AMA_OK) return;\n", output);
    } else if (strcmp(path, ".received") == 0) {
        written = fputs("void test_parallel_received(void); AmaByteView bytes; size_t code=0;\n"
            "*status=ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_view(parameters->AMA_INPUT(0)->payload,&bytes);\n"
            "if(status->code!=AMA_OK) return;\n"
            "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(1)->payload,&code);\n"
            "if(status->code!=AMA_OK) return;\n"
            "if(code!=7 || bytes.length!=3 || bytes.data[0]!=0 || bytes.data[1]!=255 || bytes.data[2]!=66) {\n"
            "*status=ama_status_make(AMA_NATIVE_FAILURE,\"Wrong parallel packet\",21); return; }\n"
            "test_parallel_received();\n", output);
    } else if (strcmp(path, ".missing") == 0) {
        written = fputs("*status=ama_status_make(AMA_NATIVE_FAILURE,\"Missing parallel completion\",27); return;\n", output);
    } else { return false; }
    assert(written != EOF);
    return true;
}

/** Independent dispatchers overlap, import a native event, and isolate a peer's accepted failure. */
static void it_runs_parallel_contexts_with_private_startup_reuse_events_and_failure(void) {
    directory("generated/project");
    native_fixture("bridge", "@<\nsource&\n..task.events\n@C\n");
    native_fixture("received", "@<\ndata\n..bytes\ncode\n..integer\n@C\n");
    native_fixture("missing", "@C\n");
    char source[4096] = {0}, consumer[32768] = {0};
    append_fixture("project/main", source, sizeof(source));
    const char *parts[] = {"consumer/state.c", "consumer/native.c", "consumer/inspect.c", "consumer/main.c"};
    for (size_t index = 0; index < sizeof(parts) / sizeof(*parts); index++) {
        size_t used = strlen(consumer);
        int written = snprintf(consumer + used, sizeof(consumer) - used, "\n#line 1 \"%s\"\n", parts[index]);
        assert(written > 0 && (size_t)written < sizeof(consumer) - used);
        append_fixture(parts[index], consumer, sizeof(consumer));
    }
    host_launcher = consumer;
    native_entry_override = parallel_native_entry;
    execute_project(source, "", true);
    expect_diagnostic("Isolated parallel failure");
    native_entry_override = NULL;
    host_launcher = NULL;
}

/** Run the complete compiler/runtime/native parallel composition. */
void run(void) {
    it_runs_parallel_contexts_with_private_startup_reuse_events_and_failure();
}

/** Successful exit requires every child-host assertion and diagnostic to match. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
