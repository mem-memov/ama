/**
 * @file
 * @brief Verify driver commands, exact option rejection and output failures.
 */
#include "../common.c"
#include <sys/stat.h>
#include <errno.h>

/**
 * Assert that the component documents verified project compilation. The case observes actual
 * production behavior through explicit fixture setup and teardown.
 */
static void it_documents_verified_project_compilation(void) {
    setup_test();
    char *arguments[] = {"ama", "--help"};
    int result = driver_run(2, arguments, output, errors);
    char message[1024];
    read_stream(output, message, sizeof(message));
    assert(result == EXIT_SUCCESS);
    assert(strstr(message, "--compile") != NULL);
    read_stream(errors, message, sizeof(message));
    assert(message[0] == '\0');
    destroy_test();
}

/**
 * Assert that the component rejects missing explicit commands on the error stream. The case
 * observes actual production behavior through explicit fixture setup and teardown.
 */
static void it_rejects_missing_explicit_commands_on_the_error_stream(void) {
    setup_test();
    char *arguments[] = {"ama", "project"};
    int result = driver_run(2, arguments, output, errors);
    char message[1024];
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_FAILURE);
    assert(strstr(message, "expected an explicit command") != NULL);
    read_stream(output, message, sizeof(message));
    assert(message[0] == '\0');
    destroy_test();
}

/**
 * Assert that the component reports failure when help cannot be written. The case observes
 * actual production behavior through explicit fixture setup and teardown.
 */
static void it_reports_failure_when_help_cannot_be_written(void) {
    setup_test();
    FILE *read_only = fopen("main.c", "r");
    assert(read_only != NULL);
    char *arguments[] = {"ama", "--help"};
    int result = driver_run(2, arguments, read_only, errors);
    assert(result == EXIT_FAILURE);
    int close_result = fclose(read_only);
    assert(close_result == 0);
    destroy_test();
}

/** Retired list type mappings cannot enter frontend loading through the CLI. */
static void it_rejects_every_retired_list_configuration_option(void) {
    const char *retired[] = {"--list-action", "--list-index", "--list-condition", "--list-outcome"};
    for (size_t index = 0; index < sizeof(retired) / sizeof(*retired); index++) {
        setup_test();
        char *arguments[] = {"ama", "--check", "unopened-root", ".main", (char *)retired[index], ".value"};
        int result = driver_run(6, arguments, output, errors);
        char message[1024];
        read_stream(errors, message, sizeof(message));
        assert(result == EXIT_FAILURE);
        assert(strstr(message, "Unknown command option") != NULL);
        read_stream(output, message, sizeof(message));
        assert(message[0] == '\0');
        destroy_test();
    }
}

/** Driver success preserves warnings as nonfatal stderr diagnostics; failed warning output prevents acknowledgement. */
static void it_publishes_empty_handler_warnings_and_propagates_warning_output_failures(void) {
    int made = mkdir("generated/project", 0700);
    assert(made == 0 || errno == EEXIST);
    FILE *main_source = fopen("generated/project/main", "wb");
    assert(main_source != NULL);
    int written = fputs("@!\nprobe\n.worker\nprobe !\n:\n", main_source);
    int closed = fclose(main_source);
    assert(written != EOF && closed == 0);
    FILE *worker_source = fopen("generated/project/worker", "wb");
    assert(worker_source != NULL);
    written = fputs("@!\n", worker_source);
    closed = fclose(worker_source);
    assert(written != EOF && closed == 0);
    setup_test();
    char *arguments[] = {"ama", "--check", "generated/project", ".main"};
    int result = driver_run(4, arguments, output, errors);
    char message[1024];
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_SUCCESS && strstr(message, ":5:1: warning[empty-body]") != NULL);
    assert(strstr(message, "error[") == NULL);
    read_stream(output, message, sizeof(message));
    assert(strstr(message, "checked .main") != NULL);
    destroy_test();
    setup_test();
    FILE *full = fopen("/dev/full", "w");
    assert(full != NULL);
    setbuf(full, NULL);
    result = driver_run(4, arguments, output, full);
    assert(result == EXIT_FAILURE && ferror(full));
    read_stream(output, message, sizeof(message));
    assert(message[0] == 0);
    (void)fclose(full);
    destroy_test();
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_publishes_empty_handler_warnings_and_propagates_warning_output_failures();
    it_rejects_every_retired_list_configuration_option();
    it_documents_verified_project_compilation();
    it_reports_failure_when_help_cannot_be_written();
    it_rejects_missing_explicit_commands_on_the_error_stream();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
