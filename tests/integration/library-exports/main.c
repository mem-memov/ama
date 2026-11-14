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
/** Explicitly register repeated success, source-backed rejection and target separation. */
void run(void) {
    it_checks_explicit_exports_with_repeatable_abi_metadata();
    it_rejects_bad_mappings_and_missing_configuration();
    it_rejects_launcher_flags_for_library_checking();
}
/** Return after actual CLI status, metadata and diagnostics match. */
int main(void) { run(); return EXIT_SUCCESS; }
