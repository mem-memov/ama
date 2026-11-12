/**
 * @file
 * @brief Record exact setup/run/teardown events for copied recursive test leaves.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef FIXTURE_HARNESS_COMMON_C
#define FIXTURE_HARNESS_COMMON_C

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#ifndef FORCE_FAILURE
#define FORCE_FAILURE 0
#endif

/**
 * Append one exact setup/run/teardown event to the copied leaf trace, making harness order
 * observable to its parent.
 */
static void record_step(const char *step) {
    FILE *observation = fopen(OBSERVATION_FILE, "a");
    assert(observation != NULL);
    int result = fputs(step, observation);
    assert(result != EOF);
    result = fclose(observation);
    assert(result == 0);
}

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    FILE *artifact = fopen("generated/started", "w");
    assert(artifact != NULL);
    int result = fclose(artifact);
    assert(result == 0);
    record_step("setup\n");
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    int result = access("generated/test", F_OK);
    assert(result == 0);
    record_step("destroy\n");
}

#endif
