#include "../common.c"

static void it_rejects_the_removed_memory_store(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_the_removed_memory_store();
}

int main(void) {
    run();
    printf("13-reject-mem-store: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
