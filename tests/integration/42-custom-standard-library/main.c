#include "../common.c"

static void it_loads_a_replacement_library_through_the_regular_native_module_path(void) {
    setup_test();
    int configured = setenv("AMA_STDLIB", "library", 1);
    assert(configured == 0);
    const char* sources[] = {"library/sys/math/add.c", "project/display.c"};
    expect_native_program_output("expected.txt", sources, 2);
    destroy_test();
}

void run(void) {
    it_loads_a_replacement_library_through_the_regular_native_module_path();
}

int main(void) {
    run();
    printf("42-custom-standard-library: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
