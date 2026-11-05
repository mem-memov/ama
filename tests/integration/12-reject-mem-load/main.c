#include "../common.c"

static void it_rejects_the_removed_memory_load(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_the_removed_memory_load();
}

int main(void) {
    run();
    printf("12-reject-mem-load: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
