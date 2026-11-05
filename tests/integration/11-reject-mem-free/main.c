#include "../common.c"

static void it_rejects_the_removed_memory_release(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_the_removed_memory_release();
}

int main(void) {
    run();
    printf("11-reject-mem-free: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
