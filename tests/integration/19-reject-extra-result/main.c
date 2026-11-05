#include "../common.c"

static void it_rejects_extra_positional_results(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_extra_positional_results();
}

int main(void) {
    run();
    printf("19-reject-extra-result: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
