/** @file
 * @brief Compose maintained task-copy fixtures and execute exact schedules through the real compiler.
 */
/** Read a maintained scenario exactly into test-owned storage. */
static void boundary_source(const char *path, char source[32768]) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t count = fread(source, 1, 32767, input);
    assert(!ferror(input) && feof(input));
    source[count] = '\0';
    int closed = fclose(input);
    assert(closed == 0);
}

/** Compose unchanged queue mechanics with the copy-specific task, owning record and explicit resume step. */
static void boundary_project(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/project/packages/test/fixture/v1.0.0/src/kind");
    const char *shared[] = {"queue", "entry", "enqueue", "pop", "dispatch"};
    const char *local[] = {"record", "task", "step"};
    char source[32768], input[256], output[256];
    for (size_t index = 0; index < sizeof(shared) / sizeof(*shared); index++) {
        int length = snprintf(input, sizeof(input), "../ready-queues/project/kind/%s", shared[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        boundary_source(input, source);
        length = snprintf(output, sizeof(output), "generated/project/packages/test/fixture/v1.0.0/src/kind/%s", shared[index]);
        assert(length > 0 && (size_t)length < sizeof(output));
        file(output, source);
    }
    for (size_t index = 0; index < sizeof(local) / sizeof(*local); index++) {
        int length = snprintf(input, sizeof(input), "project/kind/%s", local[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        boundary_source(input, source);
        length = snprintf(output, sizeof(output), "generated/project/packages/test/fixture/v1.0.0/src/kind/%s", local[index]);
        assert(length > 0 && (size_t)length < sizeof(output));
        file(output, source);
    }
    native_fixture("value", "@C\n");
    native_fixture("worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\n |7\n@>\noutput\n/test/fixture/v1.0.0/src/value\n input\nother\n/test/fixture/v1.0.0/src/value\n |99\n@C\n");
    boundary_source("project/scheduler", source);
    file("generated/project/packages/test/fixture/v1.0.0/src/scheduler", source);
    boundary_source("project/port", source);
    file("generated/project/packages/test/fixture/v1.0.0/src/port", source);
}

/** Execute an explicitly maintained copy or consumption schedule through real generated C. */
static void boundary_schedule(const char *name, const char *expected) {
    boundary_project();
    char path[256], source[32768];
    int length = snprintf(path, sizeof(path), "project/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    boundary_source(path, source);
    fprintf(stderr, "task boundary scenario: %s\n", name);
    execute_project(source, expected, true);
    expect_diagnostic("");
}
