#include "../common.c"

static void it_reads_defaults_and_retained_values_without_changing_persistent_state(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_reads_defaults_and_retained_values_without_changing_persistent_state();
}

int main(void) {
    run();
    printf("53-retained-results: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
