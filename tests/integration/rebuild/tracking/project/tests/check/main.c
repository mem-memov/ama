/**
 * @file
 * @brief Execute the copied leaf harness against the selected nested dependency value.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component reads the production value. The explicit fixture setup and
 * observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_reads_the_production_value(void) {
    int value = selected_value();
    assert(value == FIXTURE_VALUE);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_reads_the_production_value();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return 0;
}
