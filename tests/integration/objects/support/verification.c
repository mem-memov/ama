/** @file
 * @brief Check complete active closures without C execution and preserve unavailable backend boundaries.
 */
/** Native literal validity is a runtime protocol; complete failure entries and
 * the statically valid traversal plan must all finish frontend checking.
 */
static void it_checks_the_constructor_project_and_complete_failure_entries(void) {
    const char *entries[] = {".main", ".failure body", ".failure construction", ".unsupported"};
    for (size_t index = 0; index < sizeof(entries) / sizeof(*entries); index++) {
        setup_test();
        char *arguments[] = {AMA_EXECUTABLE, "--check", "project", (char *)entries[index], NULL};
        int result = run_compiler_arguments(arguments);
        char actual[128], expected[128], diagnostic[1024];
        int length = snprintf(expected, sizeof(expected), "checked %s\n", entries[index]);
        assert(length > 0 && (size_t)length < sizeof(expected));
        read_stream(output, actual, sizeof(actual));
        read_stream(errors, diagnostic, sizeof(diagnostic));
        if (result != EXIT_SUCCESS) { fprintf(stderr, "Object checking failed for %s: %s\n", entries[index], diagnostic); }
        assert(result == EXIT_SUCCESS && strcmp(actual, expected) == 0 && diagnostic[0] == '\0');
        destroy_test();
    }
}

/** Static success must not publish C that drops the currently unsupported
 * traversal body; exact backend rejection precedes creating its destination.
 */
static void it_rejects_unavailable_traversal_execution_before_publication(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--emit", "project", ".unsupported", "--out", "generated/unsupported", "--sdk", TEST_SDK, "--compile", NULL};
    int result = run_compiler_arguments(arguments);
    char acknowledgement[128], diagnostic[4096];
    read_stream(output, acknowledgement, sizeof(acknowledgement));
    read_stream(errors, diagnostic, sizeof(diagnostic));
    assert(result != EXIT_SUCCESS && acknowledgement[0] == '\0');
    assert(strstr(diagnostic, "Traversal IR requires its execution adapter") != NULL);
    assert(access("generated/unsupported", F_OK) == -1 && errno == ENOENT);
    destroy_test();
}
