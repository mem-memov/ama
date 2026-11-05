#include "../common.c"

static void it_rejects_parameters_after_results(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_parameters_after_results();
}

int main(void) {
    run();
    printf("18-reject-param-after-result: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
