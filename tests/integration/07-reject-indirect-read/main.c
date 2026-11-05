#include "../common.c"

static void it_rejects_indirect_reads(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_indirect_reads();
}

int main(void) {
    run();
    printf("07-reject-indirect-read: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
