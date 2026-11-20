/** @file
 * @brief Compile permanent-thread handoff and pinned native-affinity scenarios through the real dispatcher.
 * Maintained launcher fragments own the host protocol; generated implementation is never inspected.
 */
#include "../execution/common.c"

/** Append bounded maintained fixture text with checked I/O and explicit source locations. */
static void append_fixture(const char *path, char *buffer, size_t capacity, bool c_source) {
    size_t used = strlen(buffer);
    if (c_source) {
        int written = snprintf(buffer + used, capacity - used, "\n#line 1 \"%s\"\n", path);
        assert(written > 0 && (size_t)written < capacity - used);
        used += (size_t)written;
    }
    FILE *input = fopen(path, "rb");
    assert(input != NULL && used + 1 < capacity);
    size_t count = fread(buffer + used, 1, capacity - used - 1, input);
    assert(!ferror(input) && count < capacity - used - 1);
    int closed = fclose(input);
    assert(closed == 0);
    buffer[used + count] = 0;
}

/** Exercise pending native rights or actual thread-affine entry without retaining Ama views. */
static bool transfer_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    int written;
    if (strcmp(path, "/test/fixture/v1.0.0/src/capture") == 0) {
        written = fputs("AmaStatus test_transfer_service(const ama_t_0a13545c236dc688542e3824b23bf3848f38fdb81d6b4c289732f8a82eaa888f *,void *);\n"
            "*status=parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage,test_transfer_service,NULL);\n"
            "if(status->code!=AMA_OK) return;\n", output);
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/visit") == 0 || strcmp(path, "/test/fixture/v1.0.0/src/affine") == 0) {
        if (strcmp(path, "/test/fixture/v1.0.0/src/affine") == 0) {
            written = fputs("if(!pthread_equal(state->owner,pthread_self())) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }\n", output);
            assert(written != EOF);
        }
        written = fputs("void test_transfer_visit(size_t); state->calls++; test_transfer_visit(state->calls);\n", output);
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/missing") == 0) {
        written = fputs("*status=ama_status_make(AMA_NATIVE_FAILURE,\"Missing transferred event\",25); return;\n", output);
    } else { return false; }
    assert(written != EOF);
    return true;
}

/** Model a native provider whose entry/destruction must use the constructing OS thread. */
static const char *affine_hook(const ScaffoldType *type, AbiOperation operation) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/affine") != 0) { return NULL; }
    if (operation == ABI_DEFAULT) {
        return " { *destination=(AMA_NATIVE(State)){.address=destination,.owner=pthread_self()}; return ama_status_success(); }\n";
    }
    if (operation == ABI_DESTROY) {
        return " { return pthread_equal(state->owner,pthread_self()) ? ama_status_success() : ama_status_make(AMA_INVALID_STATE,NULL,0); }\n";
    }
    if (operation == ABI_CLONE) {
        return " { if(!pthread_equal(source->owner,pthread_self())) return ama_status_make(AMA_INVALID_STATE,NULL,0); *destination=*source; destination->address=destination; return ama_status_success(); }\n";
    }
    if (operation == ABI_LITERAL) { return " { (void)destination; (void)literal; return ama_status_make(AMA_INVALID_LITERAL,NULL,0); }\n"; }
    return NULL;
}

/** Compose the same explicit host controller with a movable or pinned resource contract. */
static void execute_transfer(bool affine) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("capture", "@<\nsource&\n//task/events\n@C\n");
    native_fixture("visit", "@C\n");
    native_fixture("missing", "@C\n");
    native_fixture("affine", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/affine.h", "#include <pthread.h>\nstruct AMA_NATIVE(State) { size_t number,calls; const void *address; pthread_t owner; };\n");
    char source[4096] = {0}, consumer[32768] = {0};
    if (affine) { strcpy(consumer, "#define TEST_AFFINE\n"); }
    append_fixture(affine ? "project/affine" : "project/main", source, sizeof(source), false);
    const char *parts[] = {"consumer/controller.c", "consumer/handoff.c", "consumer/native.c", "consumer/main.c"};
    for (size_t index = 0; index < sizeof(parts) / sizeof(*parts); index++) {
        append_fixture(parts[index], consumer, sizeof(consumer), true);
    }
    native_entry_override = transfer_entry;
    native_hook_override = affine_hook;
    host_launcher = consumer;
    execute_project(source, "", true);
    host_launcher = NULL;
    native_hook_override = NULL;
    native_entry_override = NULL;
}

/** Handoff preserves two suspended positions, root identity and pending native work. */
static void it_transfers_a_quiescent_executor_with_explicit_publication_and_no_live_borrow(void) {
    execute_transfer(false);
}

/** Host policy refuses migration rather than treating a mutex as permission to violate affinity. */
static void it_keeps_a_thread_affine_native_resource_on_its_required_executor(void) {
    execute_transfer(true);
}

/** Run explicit transfer and native-affinity boundaries independently. */
void run(void) {
    /* This successful-only composition intentionally needs no diagnostic substring probe. */
    (void)expect_diagnostic;
    it_transfers_a_quiescent_executor_with_explicit_publication_and_no_live_borrow();
    it_keeps_a_thread_affine_native_resource_on_its_required_executor();
}

/** Successful exit requires both compiled host protocols to finish with exact ownership. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
