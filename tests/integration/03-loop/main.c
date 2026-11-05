#include "../common.c"

static void it_accumulates_values_in_a_loop(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_accumulates_values_in_a_loop();
}

int main(void) {
    run();
    printf("03-loop: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
