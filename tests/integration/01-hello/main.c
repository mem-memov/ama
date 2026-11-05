#include "../common.c"

static void it_adds_two_values_and_prints_the_sum(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_adds_two_values_and_prints_the_sum();
}

int main(void) {
    run();
    printf("01-hello: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
