#include "../common.c"

static void it_rejects_recursive_module_dependencies(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_recursive_module_dependencies();
}

int main(void) {
    run();
    printf("21-reject-cycle: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
