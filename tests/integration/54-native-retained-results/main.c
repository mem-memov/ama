#include "../common.c"

static void it_reads_native_results_without_repeating_side_effects(void) {
    setup_test();
    const char* sources[] = {"project/effect.c"};
    expect_native_program_output("expected.txt", sources, 1);
    destroy_test();
}

void run(void) {
    it_reads_native_results_without_repeating_side_effects();
}

int main(void) {
    run();
    printf("54-native-retained-results: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
