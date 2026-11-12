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

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_inlines_native_entries_and_runs_label_returns_with_native_recursion();
    it_rejects_surviving_entry_bodies_even_if_the_c_program_would_run();
    it_requires_a_compiler_error_for_uninlinable_native_entries();
    it_rejects_incompatible_native_conversion_signatures();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
