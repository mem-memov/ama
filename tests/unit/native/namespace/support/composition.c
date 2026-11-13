/**
 * @file
 * @brief Compile unchanged same-basename companions with root-qualified names and planned error namespaces.
 */
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "native/toolchain/profile.h"
#include "native/status/providers/providers.h"
#include "vectors.c"

/** Wait for one normal child completion without treating interrupted waits as product failure. */
static int wait_namespace_child(pid_t process) {
    assert(process >= 0);
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/** Compile the actual emitted naming contexts with the required profile and optional exact CPP error. */
static void compile_namespace_probe(const char *expected_error) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (freopen("generated/probe.log", "wb", stderr) == NULL) { _exit(126); }
        char *arguments[] = {TEST_REAL_COMPILER, AMA_TOOLCHAIN_REQUIRED_FLAGS,
            "-I" TEST_ROOT "/src", "-I" TEST_ROOT "/tests/unit/native/namespace/project",
            "-O2", "generated/probe.c", "-o", "generated/probe", NULL};
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int code = wait_namespace_child(process);
    if (expected_error == NULL) {
        assert(code == 0);
        process = fork();
        assert(process >= 0);
        if (process == 0) { execl("generated/probe", "generated/probe", (char *)NULL); _exit(127); }
        assert(wait_namespace_child(process) == 0);
    } else {
        assert(code != 0);
        FILE *errors = fopen("generated/probe.log", "rb");
        assert(errors != NULL);
        char text[8192];
        size_t count = fread(text, 1, sizeof(text) - 1, errors);
        assert(count < sizeof(text) - 1 && !ferror(errors) && fclose(errors) == 0);
        text[count] = '\0';
        assert(strstr(text, expected_error) != NULL);
    }
}

/** Emit one unchanged companion inclusion and require both temporary SDK macros to disappear. */
static void emit_companion(FILE *output, const NativeStatusProviders *providers,
    const IdentityName *name, const char *path) {
    uint16_t namespace_id = 0;
    assert(native_status_provider_find(providers, name, &namespace_id).code == AMA_OK);
    assert(native_namespace_begin(output, name));
    assert(native_namespace_status(output, namespace_id));
    assert(fprintf(output, "#include \"%s\"\n", path) > 0);
    assert(native_namespace_end(output));
    assert(fputs("#if defined(AMA_NATIVE) || defined(AMA_NATIVE_ERROR)\n#error Leaked native context\n#endif\n", output) != EOF);
}

/** Compile header-first/reverse-discovery and body-forward composition; execute both sets of hooks. */
static void it_composes_root_distinct_companions_without_symbol_or_macro_leaks(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *project = register_namespace_vector(&registry, 0);
    const IdentityName *standard = register_namespace_vector(&registry, 2);
    const IdentityName *inventory[] = {standard, project};
    NativeStatusProviders providers;
    native_status_providers_init(&providers, sources.allocator);
    assert(native_status_providers_build(&providers, inventory, 2).code == AMA_OK);
    FILE *output = fopen("generated/probe.c", "wb");
    assert(output != NULL);
    assert(fputs("#include <assert.h>\n#include <string.h>\n", output) != EOF);
    emit_companion(output, &providers, standard, "standard/module.h");
    emit_companion(output, &providers, project, "project/module.h");
    emit_companion(output, &providers, project, "project/module.c");
    emit_companion(output, &providers, standard, "standard/module.c");
    assert(fputs("int main(void) {\n", output) != EOF);
    const IdentityName *names[] = {project, standard};
    const char *messages[] = {"project", "standard"};
    const int seeds[] = {21,34};
    for (size_t index = 0; index < 2; index++) {
        const char *prefix = names[index]->internal_name;
        assert(fprintf(output,
            "{ %s_State state, copy;\n"
            "assert(%s_initialize(&state).code == AMA_OK && state.value == %d);\n"
            "assert(%s_clone(&copy, &state).code == AMA_OK);\n"
            "state.value++; assert(copy.value == %d);\n"
            "AmaStatus failure = %s_failure();\n"
            "assert(failure.code == ((UINT32_C(%zu) << 16) | 7));\n"
            "assert(failure.message_length == %zu && memcmp(failure.message, \"%s\", %zu) == 0);\n"
            "size_t counts[3] = {0}; AmaStatus once = %s_evaluate_once(counts);\n"
            "assert(once.code == failure.code && counts[0] == 1 && counts[1] == 1 && counts[2] == 1); }\n",
            prefix, prefix, seeds[index], prefix, seeds[index], prefix, index + 1,
            strlen(messages[index]), messages[index], strlen(messages[index]), prefix) > 0);
    }
    assert(fputs("return 0; }\n", output) != EOF && fclose(output) == 0);
    compile_namespace_probe(NULL);
    native_status_providers_destroy(&providers);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Require precise diagnostics for missing, duplicated and foreign active preprocessing contexts. */
static void it_rejects_missing_nested_and_unclean_native_contexts(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *name = register_namespace_vector(&registry, 0);
    const char *errors[] = {"requires an active naming context", "provider is already active",
        "AMA_NATIVE context is already active", "AMA_NATIVE context is already active"};
    for (size_t selected = 0; selected < 4; selected++) {
        FILE *output = fopen("generated/probe.c", "wb");
        assert(output != NULL);
        if (selected == 3) { assert(fputs("#define AMA_NATIVE_ERROR foreign_context\n", output) != EOF); }
        if (selected != 0) { assert(native_namespace_begin(output, name)); }
        assert(native_namespace_status(output, 1));
        if (selected == 1) { assert(native_namespace_status(output, 1)); }
        if (selected == 2) { assert(native_namespace_begin(output, name)); }
        assert(native_namespace_end(output));
        assert(fputs("int main(void) { return 0; }\n", output) != EOF && fclose(output) == 0);
        compile_namespace_probe(errors[selected]);
    }
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Invalid provider emission is side-effect free; failed streams never claim a complete context. */
static void it_checks_error_context_arguments_and_output_failure(void) {
    FILE *output = tmpfile();
    assert(output != NULL);
    assert(!native_namespace_status(output, 0) && ftell(output) == 0);
    assert(!native_namespace_status(NULL, 1));
    assert(fclose(output) == 0);
    output = fopen("/dev/full", "wb");
    assert(output != NULL && setvbuf(output, NULL, _IONBF, 0) == 0);
    assert(!native_namespace_status(output, 1) && ferror(output));
    fclose(output);
}
