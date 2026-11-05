#include "../common.c"

static void it_disables_default_modules_when_the_library_path_is_empty(void) {
    setup_test();
    int configured = setenv("AMA_STDLIB", "", 1);
    assert(configured == 0);
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_disables_default_modules_when_the_library_path_is_empty();
}

int main(void) {
    run();
    printf("41-disable-standard-library: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
