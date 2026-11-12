/**
 * @file
 * @brief Check nominal incompatibility across real Unicode module fixtures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component renders distinct unicode module types and both declarations. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_renders_distinct_unicode_module_types_and_both_declarations(void) {
    setup_test();
    SemanticStatus status = semantic_analyze(&semantic);
    assert(status == SEMANTIC_INVALID && !semantic.resolved);
    expect_diagnostic();
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_renders_distinct_unicode_module_types_and_both_declarations();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
