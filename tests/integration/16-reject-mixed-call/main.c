#include "../common.c"

static void it_rejects_mixed_argument_modes(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_mixed_argument_modes();
}

int main(void) {
    run();
    printf("16-reject-mixed-call: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
