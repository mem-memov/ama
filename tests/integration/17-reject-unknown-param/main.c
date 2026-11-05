#include "../common.c"

static void it_rejects_unknown_named_parameters(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_unknown_named_parameters();
}

int main(void) {
    run();
    printf("17-reject-unknown-param: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
