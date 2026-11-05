#include "../common.c"

static void it_rejects_native_implementations_until_supported(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_native_implementations_until_supported();
}

int main(void) {
    run();
    printf("25-reject-native-annotation: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
