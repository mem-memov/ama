#include "../common.c"

static void it_keeps_case_sensitive_headers_and_private_c_helpers_independent(void) {
    setup_test();
    const char* sources[] = {"project/math/value.c", "project/math_Value.c"};
    expect_native_program_output("expected.txt", sources, 2);
    destroy_test();
}

void run(void) {
    it_keeps_case_sensitive_headers_and_private_c_helpers_independent();
}

int main(void) {
    run();
    printf("39-native-case: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
