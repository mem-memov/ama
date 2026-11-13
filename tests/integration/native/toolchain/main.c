/**
 * @file
 * @brief Compile supported native-entry probes, inspect symbols and execute the accepted stackless fixture.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component inlines native entries and runs label returns with native recursion.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_inlines_native_entries_and_runs_label_returns_with_native_recursion(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/positive.c", options[index]);
        assert(result == 0);
        AmaSymbolCheck symbols = inspect_entry();
        assert(symbols == AMA_SYMBOLS_CLEAR);
        static const unsigned char helper_name[] = "optional_inline_helper";
        const AmaByteView helper = {helper_name, sizeof(helper_name) - 1};
        assert(inspect_entries(&helper, 1) == AMA_SYMBOLS_ENTRY_REMAINS);
        link_and_execute();
    }
}

/**
 * Assert that the component rejects surviving entry bodies even if the c program would run. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_rejects_surviving_entry_bodies_even_if_the_c_program_would_run(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/noninline.c", options[index]);
        assert(result == 0);
        AmaSymbolCheck symbols = inspect_entry();
        assert(symbols == AMA_SYMBOLS_ENTRY_REMAINS);
    }
}

/** A working C call dispatcher still violates the Ama execution boundary. */
static void it_rejects_ama_body_dispatch_through_a_c_function(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/ama-call.c", options[index]);
        assert(result == 0 && inspect_entry() == AMA_SYMBOLS_ENTRY_REMAINS);
    }
}

/**
 * Assert that the component requires a compiler error for uninlinable native entries. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_requires_a_compiler_error_for_uninlinable_native_entries(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/uninlinable.c", options[index]);
        assert(result != 0);
    }
}

/**
 * Assert that the component rejects incompatible native conversion signatures. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_rejects_incompatible_native_conversion_signatures(void) {
    int result = compile_fixture("project/wrong-signature.c", "-O0");
    assert(result != 0);
}

/** The header must explicitly reject unsupported compiler identity, target and C mode.
 * Each log is reset so a prior expected rejection cannot satisfy a later assertion.
 */
static void it_rejects_unsupported_backend_configuration_with_explicit_diagnostics(void) {
    int baseline = compile_fixture("project/profile.c", "-O0");
    assert(baseline == 0 && inspect_entry() == AMA_SYMBOLS_CLEAR);
    link_and_execute();
    const char *overrides[] = {"-D__clang__=1", "-U__linux__", "-std=c99"};
    const char *messages[] = {"requires verified GCC 15.2.0",
        "requires C11 on x86_64 Linux", "requires C11 on x86_64 Linux"};
    for (size_t index = 0; index < sizeof(overrides) / sizeof(*overrides); index++) {
        FILE *errors = fopen("generated/errors.log", "wb");
        assert(errors != NULL && fclose(errors) == 0);
        char *arguments[] = {TEST_REAL_COMPILER, AMA_TOOLCHAIN_REQUIRED_FLAGS,
            "-I" TEST_ROOT "/src", (char *)overrides[index], "-c", "project/profile.c",
            "-o", "generated/module.o", NULL};
        int result = run_tool(arguments, "generated/compiler.log");
        assert(result != 0);
        errors = fopen("generated/errors.log", "rb");
        assert(errors != NULL);
        char text[8192];
        size_t length = fread(text, 1, sizeof(text) - 1, errors);
        assert(!ferror(errors) && length < sizeof(text) - 1 && fclose(errors) == 0);
        text[length] = '\0';
        assert(strstr(text, messages[index]) != NULL);
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_inlines_native_entries_and_runs_label_returns_with_native_recursion();
    it_rejects_surviving_entry_bodies_even_if_the_c_program_would_run();
    it_rejects_ama_body_dispatch_through_a_c_function();
    it_requires_a_compiler_error_for_uninlinable_native_entries();
    it_rejects_incompatible_native_conversion_signatures();
    it_rejects_unsupported_backend_configuration_with_explicit_diagnostics();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
