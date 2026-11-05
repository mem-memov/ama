#include "../common.c"

static void it_rejects_an_ama_body_in_a_native_interface(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_an_ama_body_in_a_native_interface();
}

int main(void) {
    run();
    printf("30-reject-native-body: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
