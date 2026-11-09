#include "../common.c"

static void it_does_not_inherit_loop_context_from_the_caller(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_does_not_inherit_loop_context_from_the_caller();
}

int main(void) {
    run();
    printf("51-reject-cross-module-exit: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
