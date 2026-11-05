#include "../common.c"

static void it_rejects_project_modules_that_shadow_the_standard_library(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_project_modules_that_shadow_the_standard_library();
}

int main(void) {
    run();
    printf("40-reject-library-shadow: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
