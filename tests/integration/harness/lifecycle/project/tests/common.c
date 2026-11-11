#ifndef FIXTURE_HARNESS_COMMON_C
#define FIXTURE_HARNESS_COMMON_C

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#ifndef FORCE_FAILURE
#define FORCE_FAILURE 0
#endif

static void record_step(const char *step) {
    FILE *observation = fopen(OBSERVATION_FILE, "a");
    assert(observation != NULL);
    int result = fputs(step, observation);
    assert(result != EOF);
    result = fclose(observation);
    assert(result == 0);
}

static void setup_test(void) {
    FILE *artifact = fopen("generated/started", "w");
    assert(artifact != NULL);
    int result = fclose(artifact);
    assert(result == 0);
    record_step("setup\n");
}

static void destroy_test(void) {
    int result = access("generated/test", F_OK);
    assert(result == 0);
    record_step("destroy\n");
}

#endif
