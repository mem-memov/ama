#include "../common.c"

static void it_rejects_a_call_without_its_marker(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_a_call_without_its_marker();
}

int main(void) {
    run();
    printf("50-reject-bare-call: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
