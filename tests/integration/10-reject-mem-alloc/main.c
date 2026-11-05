#include "../common.c"

static void it_rejects_the_removed_memory_allocator(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_the_removed_memory_allocator();
}

int main(void) {
    run();
    printf("10-reject-mem-alloc: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
