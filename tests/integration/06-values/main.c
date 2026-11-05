#include "../common.c"

static void it_copies_values_and_resets_callee_defaults_and_locals(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_copies_values_and_resets_callee_defaults_and_locals();
}

int main(void) {
    run();
    printf("06-values: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
