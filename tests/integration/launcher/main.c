#include "../common.c"

static void it_exposes_implementation_status_through_the_real_launcher(void) {
    setup_test();
    int result = run_compiler("--help");
    char message[512];
    read_stream(output, message, sizeof(message));
    assert(result == EXIT_SUCCESS);
    assert(strstr(message, "not implemented") != NULL);
    read_stream(errors, message, sizeof(message));
    assert(message[0] == '\0');
    destroy_test();
}

static void it_does_not_succeed_for_an_unimplemented_project_build(void) {
    setup_test();
    int result = run_compiler("project");
    char message[512];
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_FAILURE);
    assert(strstr(message, "not implemented") != NULL);
    read_stream(output, message, sizeof(message));
    assert(message[0] == '\0');
    destroy_test();
}

void run(void) {
    it_exposes_implementation_status_through_the_real_launcher();
    it_does_not_succeed_for_an_unimplemented_project_build();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
