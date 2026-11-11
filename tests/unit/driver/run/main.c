#include "../common.c"

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

void run(void) {
    it_documents_verified_project_compilation();
    it_reports_failure_when_help_cannot_be_written();
    it_rejects_missing_explicit_commands_on_the_error_stream();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
