/** @file
 * @brief Validate actual library CLI metadata and reject invalid inputs without executable emission.
 */
#include "../common.c"
/** Run one exact library request with explicit standard authority and fresh streams. */
static int check_library(const char *configuration) {
    char *arguments[] = {AMA_EXECUTABLE, "--library-check", "project", (char *)configuration,
        "--standard-root", TEST_STANDARD_ROOT, NULL};
    return run_compiler_arguments(arguments);
}
/** Two checked resolutions produce identical machine-readable public metadata. */
static void it_checks_explicit_exports_with_repeatable_abi_metadata(void) {
    char first[16384], second[16384], diagnostic[4096];
    setup_test();
    int result = check_library("component.exports");
    read_stream(errors, diagnostic, sizeof(diagnostic));
    if (result != EXIT_SUCCESS) { fprintf(stderr, "%s", diagnostic); }
    assert(result == EXIT_SUCCESS && diagnostic[0] == '\0');
    read_stream(output, first, sizeof(first));
    assert(strstr(first, "\"format\":\"ama-c-build-1\"") != NULL);
    assert(strstr(first, "\"component\":\"example.component\"") != NULL);
    assert(strstr(first, "\"profile\":\"failure-v1\"") != NULL);
    assert(strstr(first, "\"profile\":\"list-v1\"") != NULL);
    assert(strstr(first, "\"field\":\"message text\",\"schema\":\"message\",\"input\":true,\"ordinal\":1") != NULL);
    assert(strstr(first, "unselected") == NULL && strstr(first, "launcher") == NULL);
    destroy_test();
    setup_test();
    result = check_library("component.exports");
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert(result == EXIT_SUCCESS && diagnostic[0] == '\0');
    read_stream(output, second, sizeof(second));
    assert(strcmp(first, second) == 0);
    destroy_test();
}
/** Wrong fields and nonexistent configuration files publish no successful metadata. */
static void it_rejects_bad_mappings_and_missing_configuration(void) {
    setup_test();
    int result = check_library("bad.exports");
    char diagnostic[4096], actual[128];
    read_stream(errors, diagnostic, sizeof(diagnostic));
    read_stream(output, actual, sizeof(actual));
    assert(result != EXIT_SUCCESS && actual[0] == '\0');
    assert(strstr(diagnostic, "bad.exports:9:1: error[type-mismatch]") != NULL);
    assert(strstr(diagnostic, "mapped interface field") != NULL);
    destroy_test();
    setup_test();
    result = check_library("absent.exports");
    read_stream(errors, diagnostic, sizeof(diagnostic));
    read_stream(output, actual, sizeof(actual));
    assert(result != EXIT_SUCCESS && actual[0] == '\0' && strstr(diagnostic, "io-failure") != NULL);
    destroy_test();
}
/** Checking cannot silently select executable compilation or an output directory. */
static void it_rejects_launcher_flags_for_library_checking(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--library-check", "project", "component.exports", "--compile", NULL};
    int result = run_compiler_arguments(arguments);
    char diagnostic[4096], actual[128];
    read_stream(errors, diagnostic, sizeof(diagnostic));
    read_stream(output, actual, sizeof(actual));
    assert(result != EXIT_SUCCESS && actual[0] == '\0' && diagnostic[0] != '\0');
    destroy_test();
}
/** The actual CLI emits a standalone C11 header, with no SDK include path or executable emission. */
static void it_emits_a_public_header_that_compiles_without_the_compiler_sdk(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--library-header", "project", "component.exports",
        "--standard-root", TEST_STANDARD_ROOT, NULL};
    int result = run_compiler_arguments(arguments);
    char diagnostic[8192];
    read_stream(errors, diagnostic, sizeof(diagnostic));
    if (result != EXIT_SUCCESS) { fprintf(stderr, "%s", diagnostic); }
    assert(result == EXIT_SUCCESS && diagnostic[0] == 0);
    int seek = fseek(output, 0, SEEK_SET); assert(seek == 0);
    FILE *header = fopen("generated/public.h", "wb"); assert(header != NULL);
    for (;;) {
        unsigned char bytes[4096];
        size_t length = fread(bytes, 1, sizeof(bytes), output);
        assert(!ferror(output));
        size_t written = fwrite(bytes, 1, length, header); assert(written == length);
        if (length == 0) { break; }
    }
    int closed = fclose(header); assert(closed == 0);
    FILE *consumer = fopen("generated/consumer.c", "wb"); assert(consumer != NULL);
    int written = fputs("/** @file Independent C11 consumer of the actual CLI header. */\n#include \"public.h\"\nint main(void){return 0;}\n", consumer);
    closed = fclose(consumer); assert(written != EOF && closed == 0);
    destroy_test();
    setup_test();
    pid_t child = fork(); assert(child >= 0);
    if (child == 0) {
        if (dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        execlp(TEST_C_COMPILER, TEST_C_COMPILER, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic-errors", "-c", "generated/consumer.c", "-o", "generated/consumer.o", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    read_stream(errors, diagnostic, sizeof(diagnostic));
    if (diagnostic[0] != 0) { fprintf(stderr, "%s", diagnostic); }
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0 && diagnostic[0] == 0);
    destroy_test();
}

/** Explicitly register repeated success, source-backed rejection and target separation. */
void run(void) {
    it_emits_a_public_header_that_compiles_without_the_compiler_sdk();
    it_checks_explicit_exports_with_repeatable_abi_metadata();
    it_rejects_bad_mappings_and_missing_configuration();
    it_rejects_launcher_flags_for_library_checking();
}
/** Return after actual CLI status, metadata and diagnostics match. */
int main(void) { run(); return EXIT_SUCCESS; }
