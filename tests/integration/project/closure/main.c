/**
 * @file
 * @brief Verify real-file dependency failure chains and exact rendered diagnostics.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component renders the complete unicode dependency chain. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_renders_the_complete_unicode_dependency_chain(void) {
    setup_test();
    ModuleRoot root = {"project", NULL, 0, MODULE_ROOT_PROJECT};
    ProjectStatus status = project_load(&project, &root, 1, ".main");
    assert(status == PROJECT_MISSING && !project.valid);
    expect_diagnostic("expected-error.txt");
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_renders_the_complete_unicode_dependency_chain();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
