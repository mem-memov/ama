#include "../common.c"

static void it_rejects_indirect_writes(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_indirect_writes();
}

int main(void) {
    run();
    printf("08-reject-indirect-write: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
