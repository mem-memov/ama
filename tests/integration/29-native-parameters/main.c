#include "../common.c"

static void it_passes_parameters_to_a_native_function_without_results(void) {
    setup_test();
    const char* sources[] = {"project/log.c"};
    expect_native_program_output("expected.txt", sources, sizeof(sources) / sizeof(sources[0]));
    destroy_test();
}

void run(void) {
    it_passes_parameters_to_a_native_function_without_results();
}

int main(void) {
    run();
    printf("29-native-parameters: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
