#include "../common.c"

static void it_rejects_the_old_standalone_exit_symbol(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_the_old_standalone_exit_symbol();
}

int main(void) {
    run();
    printf("52-reject-standalone-bang: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
