/**
 * @file
 * @brief Count exact recorded compile/link invocations in incremental-build tests.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_BUILD_TRACE_C
#define AMA_TEST_BUILD_TRACE_C

/**
 * Replace only the test-owned compiler trace with an empty file before measuring the next build.
 */
static void reset_trace(void) {
    write_file(trace_path, "");
}

/**
 * Read complete trace records and count only exact kind plus separator matches; prefixes do not
 * accidentally count another operation.
 */
static size_t trace_count(const char *kind) {
    FILE *trace = fopen(trace_path, "r");
    assert(trace != NULL);
    char record[4096];
    size_t count = 0;
    while (fgets(record, sizeof(record), trace) != NULL) {
        if (strncmp(record, kind, strlen(kind)) == 0 && record[strlen(kind)] == ' ') {
            count++;
        }
    }
    assert(!ferror(trace));
    int result = fclose(trace);
    assert(result == 0);
    return count;
}

/**
 * Require exact measured compile and link counts after one controlled build step, independently
 * of program output.
 */
static void expect_invocations(size_t compilations, size_t links) {
    size_t actual_compilations = trace_count("compile");
    size_t actual_links = trace_count("link");
    assert(actual_compilations == compilations);
    assert(actual_links == links);
}

#endif
