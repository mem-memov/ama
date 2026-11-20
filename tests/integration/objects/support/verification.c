/** @file
 * @brief Check complete active closures and execute live traversal through the real CLI.
 */
/** Native literal validity is a runtime protocol; complete failure entries and
 * the statically valid traversal plan must all finish frontend checking.
 */
static void it_checks_the_constructor_project_and_complete_failure_entries(void) {
    const char *entries[] = {"/test/fixture/v1.0.0/src/main", "/test/fixture/v1.0.0/src/failure body", "/test/fixture/v1.0.0/src/failure construction", "/test/fixture/v1.0.0/src/traversal"};
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

/** Actual owning selection calls its retained target and releases both the
 * copied native resource and immutable seed through complete runtime cleanup.
 */
static void it_executes_live_traversal_with_complete_resource_cleanup(void) {
    compile_and_run_object("/test/fixture/v1.0.0/src/traversal", "count=1\ndrop=1\ndrop=0\n", NULL, false);
}
