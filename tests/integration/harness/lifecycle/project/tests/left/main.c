/**
 * @file
 * @brief Exercise successful or deliberately failed left-leaf cleanup under the copied harness.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#define OBSERVATION_FILE "../../generated/left"
#include "../common.c"

/**
 * Assert that the component executes between setup and destruction. The explicit fixture setup
 * and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_executes_between_setup_and_destruction(void) {
    setup_test();
    record_step("run\n");
    assert(!FORCE_FAILURE);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_executes_between_setup_and_destruction();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return 0;
}
