#include "../common.c"

static void it_passes_values_and_result_pointers_with_defaults(void) {
    setup_test();
    const char* sources[] = {"project/math/stats.c"};
    expect_native_program_output("expected.txt", sources, sizeof(sources) / sizeof(sources[0]));
    destroy_test();
}

void run(void) {
    it_passes_values_and_result_pointers_with_defaults();
}

int main(void) {
    run();
    printf("26-native-calls: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
