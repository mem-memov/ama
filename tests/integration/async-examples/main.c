/** @file
 * @brief Verify the published asynchronous examples and fresh-output policy through their actual entry commands.
 */
#include "common.c"

/** Explicit queue membership, completion delivery and cancellation produce two coherent ordinary results. */
static void it_runs_the_published_typed_queue_completion_and_cancellation_example(void) {
    char *build[] = {AMA_EXECUTABLE, "--emit", TEST_REPOSITORY_ROOT "/examples/async", ".main", "--out", "generated/ama",
        "--sdk", TEST_SDK, "--standard-root", TEST_STANDARD_ROOT, "--compile", NULL};
    command(build, true, NULL);
    char *run[] = {"generated/ama/program", NULL};
    command(run, true, "completed\ncancelled\n");
}

/** The actual product CLI builds a reloadable component, then an SDK-free host retires every callback and value. */
static void it_runs_the_published_native_host_and_preserves_an_existing_destination(void) {
    char *build[] = {"python3", TEST_REPOSITORY_ROOT "/tools/examples/async_host/run.py", "--compiler", AMA_EXECUTABLE,
        "--out", "generated/host", NULL};
    command(build, true, "asynchronous component lifetime accepted\n");
    FILE *marker = fopen("generated/host/keep.txt", "wb");
    assert(marker != NULL);
    int written = fputs("keep\n", marker);
    int closed = fclose(marker);
    assert(written != EOF && closed == 0);
    command(build, false, "");
    marker = fopen("generated/host/keep.txt", "rb");
    assert(marker != NULL);
    char bytes[16] = {0};
    size_t count = fread(bytes, 1, sizeof(bytes), marker);
    assert(!ferror(marker) && count == 5 && memcmp(bytes, "keep\n", 5) == 0);
    closed = fclose(marker);
    assert(closed == 0);
}

/** Reject incomplete arguments and malformed producer identities before emitting an ambiguous C binding. */
static void it_rejects_incomplete_commands_and_duplicate_public_metadata(void) {
    char *missing[] = {"python3", TEST_REPOSITORY_ROOT "/tools/examples/async_host/run.py", NULL};
    command(missing, false, "");
    char *metadata[] = {"python3", "-c",
        "import sys; sys.path.insert(0,sys.argv[1]); import model; "
        "model.named([{'name':'run','identity':'one'},{'name':'run','identity':'two'}])",
        TEST_REPOSITORY_ROOT "/tools/examples/async_host", NULL};
    command(metadata, false, "");
}

/** Run the real examples and their documented failure boundaries. */
void run(void) {
    it_runs_the_published_typed_queue_completion_and_cancellation_example();
    it_runs_the_published_native_host_and_preserves_an_existing_destination();
    it_rejects_incomplete_commands_and_duplicate_public_metadata();
}
/** Report success only after the public entry paths and preservation checks complete. */
int main(void) { run(); return EXIT_SUCCESS; }
