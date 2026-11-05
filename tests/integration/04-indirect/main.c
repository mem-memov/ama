#include "../common.c"

static void it_rejects_address_assignment(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_address_assignment();
}

int main(void) {
    run();
    printf("04-indirect: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
