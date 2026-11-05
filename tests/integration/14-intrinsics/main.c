#include "../common.c"

static void it_runs_arithmetic_comparison_logic_and_division_errors(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_runs_arithmetic_comparison_logic_and_division_errors();
}

int main(void) {
    run();
    printf("14-intrinsics: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
