/**
 * @file
 * @brief Own separate borrowed-driver output/error streams and bounded result reading.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_DRIVER_COMMON_C
#define AMA_TEST_DRIVER_COMMON_C

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/driver.h"

static FILE *output;
static FILE *errors;

/**
 * Initialize this leaf fixture before its assertions; the enclosing file contract identifies
 * owned streams, source storage or model state. Setup failure is an assertion, never an accepted
 * product failure.
 */
static void setup_test(void) {
    output = tmpfile();
    errors = tmpfile();
    assert(output != NULL);
    assert(errors != NULL);
}

/**
 * Release or reset this leaf fixture after observations, in reverse borrow order. Counted owners
 * assert zero remaining live allocations; assertions remain enabled by the leaf harness.
 */
static void destroy_test(void) {
    int output_result = fclose(output);
    int errors_result = fclose(errors);
    assert(output_result == 0);
    assert(errors_result == 0);
}

/**
 * Rewind a borrowed stream and read a bounded NUL-terminated observation. Require spare capacity
 * and successful IO; the caller owns the buffer and stream.
 */
static void read_stream(FILE *stream, char *buffer, size_t capacity) {
    int seek_result = fseek(stream, 0, SEEK_SET);
    assert(seek_result == 0);
    size_t length = fread(buffer, 1, capacity - 1, stream);
    assert(!ferror(stream));
    assert(length < capacity - 1);
    buffer[length] = '\0';
}

#endif
