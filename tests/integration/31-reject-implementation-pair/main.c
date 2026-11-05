#include "../common.c"

static void it_rejects_two_implementation_annotations(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_two_implementation_annotations();
}

int main(void) {
    run();
    printf("31-reject-implementation-pair: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
