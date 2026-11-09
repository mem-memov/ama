#include "../common.c"

static void it_rejects_execution_and_retained_arrows_in_one_call(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_execution_and_retained_arrows_in_one_call();
}

int main(void) {
    run();
    printf("56-reject-mixed-result-arrows: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
