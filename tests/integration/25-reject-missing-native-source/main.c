#include "../common.c"

static void it_rejects_a_native_module_without_its_companion(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_a_native_module_without_its_companion();
}

int main(void) {
    run();
    printf("25-reject-missing-native-source: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
