#include "../common.c"

static void it_runs_a_native_entry_without_parameters_or_results(void) {
    setup_test();
    const char* sources[] = {"project/main.c"};
    expect_native_program_output("expected.txt", sources, sizeof(sources) / sizeof(sources[0]));
    destroy_test();
}

void run(void) {
    it_runs_a_native_entry_without_parameters_or_results();
}

int main(void) {
    run();
    printf("27-native-entry: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
