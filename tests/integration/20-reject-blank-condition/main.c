#include "../common.c"

static void it_rejects_a_blank_between_a_condition_and_its_marker(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_a_blank_between_a_condition_and_its_marker();
}

int main(void) {
    run();
    printf("20-reject-blank-condition: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
