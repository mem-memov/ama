#include "../common.c"

static void it_keeps_flat_and_nested_module_paths_distinct(void) {
    setup_test();
    const char* sources[] = {"project/math/add.c", "project/math_add.c"};
    expect_native_program_output("expected.txt", sources, 2);
    destroy_test();
}

void run(void) {
    it_keeps_flat_and_nested_module_paths_distinct();
}

int main(void) {
    run();
    printf("35-distinct-module-paths: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
