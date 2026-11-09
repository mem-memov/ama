#include "../common.c"

static void it_rejects_parameters_in_retained_result_calls(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_parameters_in_retained_result_calls();
}

int main(void) {
    run();
    printf("55-reject-read-parameters: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
