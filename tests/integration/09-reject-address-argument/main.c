#include "../common.c"

static void it_rejects_address_arguments(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_address_arguments();
}

int main(void) {
    run();
    printf("09-reject-address-argument: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
