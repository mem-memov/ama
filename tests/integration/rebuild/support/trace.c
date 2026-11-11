#ifndef AMA_TEST_BUILD_TRACE_C
#define AMA_TEST_BUILD_TRACE_C

static void reset_trace(void) {
    write_file(trace_path, "");
}

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

static void expect_invocations(size_t compilations, size_t links) {
    size_t actual_compilations = trace_count("compile");
    size_t actual_links = trace_count("link");
    assert(actual_compilations == compilations);
    assert(actual_links == links);
}

#endif
